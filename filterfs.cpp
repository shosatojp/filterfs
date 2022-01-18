/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>
  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/
#define FUSE_USE_VERSION 31
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif
#include "filter.hpp"
#include <filesystem>
#include <optional>
// #include "passthrough_helpers.h"

void fullpath2(char *fpath, const char *root, const char *path) {
    char *fp = fpath;
    for (const char *rp = root; *rp != '\0'; rp++, fp++) {
        *fp = *rp;
    }
    if (*(fp - 1) == '/') {
        fp--;
    }
    if (*path != '/') {
        *(fp++) = '/';
    }
    for (const char *pp = path; *pp != '\0'; pp++, fp++) {
        *fp = *pp;
    }
    *(fp++) = '\0';
}

// std::optional<std::string> fullpath(const std::string &root, const std::string &path) {
//     std::stringstream ss;
//     if (root[root.length() - 1] == '/') {
//         ss << root.substr(0, root.length() - 1);
//     } else {
//         ss << root;
//     }
//     ss << '/';
//     if (path[0] == '/') {
//         ss << path.substr(1);
//     } else {
//         ss << path;
//     }
//     return ss.str();
// }

static enum fuse_fill_dir_flags fill_dir_plus = (enum fuse_fill_dir_flags)0;
static path_filter filter;
static char source_root[PATH_MAX];

static void *xmp_init(struct fuse_conn_info *conn,
                      struct fuse_config *cfg) {
    (void)conn;
    cfg->use_ino = 1;
    /* Pick up changes from lower filesystem right away. This is
           also necessary for better hardlink support. When the kernel
           calls the unlink() handler, it does not know the inode of
           the to-be-removed entry and can therefore not invalidate
           the cache of the associated inode - resulting in an
           incorrect st_nlink value being reported for any remaining
           hardlinks to this inode. */
    cfg->entry_timeout = 0;
    cfg->attr_timeout = 0;
    cfg->negative_timeout = 0;
    return NULL;
}
static int xmp_getattr(const char *path, struct stat *stbuf,
                       struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath2(fpath, source_root, path);
    if (lstat(fpath, stbuf) == -1)
        return -errno;
    return 0;
}
static int xmp_access(const char *path, int mask) {
    char fpath[PATH_MAX];
    fullpath2(fpath, source_root, path);
    if (access(fpath, mask) == -1)
        return -errno;
    return 0;
}
static int xmp_readlink(const char *path, char *buf, size_t size) {
    char fpath[PATH_MAX];
    fullpath2(fpath, source_root, path);
    int res;
    if ((res = readlink(fpath, buf, size - 1)) == -1)
        return -errno;
    buf[res] = '\0';
    return 0;
}
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags) {
    char fpath[PATH_MAX];
    fullpath2(fpath, source_root, path);
    DIR *dp = opendir(fpath);
    if (dp == NULL)
        return -errno;
    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        char fpath2[PATH_MAX];
        fullpath2(fpath2, fpath, de->d_name);
        if (!filter.contains(fpath2)) {
            continue;
        }
        if (filler(buf, de->d_name, &st, 0, fill_dir_plus))
            break;
    }
    closedir(dp);
    return 0;
}
static int xmp_open(const char *path, struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath2(fpath, source_root, path);
    int res;
    if ((res = open(fpath, fi->flags)) == -1)
        return -errno;
    fi->fh = res;
    return 0;
}
static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath2(fpath, source_root, path);

    int fd;
    int res;
    if (fi == NULL)
        fd = open(fpath, O_RDONLY);
    else
        fd = fi->fh;

    if (fd == -1)
        return -errno;
    res = pread(fd, buf, size, offset);
    if (res == -1)
        res = -errno;
    if (fi == NULL)
        close(fd);
    return res;
}
// static int xmp_statfs(const char *path, struct statvfs *stbuf) {
//     int res;
//     res = statvfs(path, stbuf);
//     if (res == -1)
//         return -errno;
//     return 0;
// }
static int xmp_release(const char *path, struct fuse_file_info *fi) {
    close(fi->fh);
    return 0;
}
#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, const char *value,
                        size_t size, int flags) {
    int res = lsetxattr(path, name, value, size, flags);
    if (res == -1)
        return -errno;
    return 0;
}
static int xmp_getxattr(const char *path, const char *name, char *value,
                        size_t size) {
    int res = lgetxattr(path, name, value, size);
    if (res == -1)
        return -errno;
    return res;
}
static int xmp_listxattr(const char *path, char *list, size_t size) {
    int res = llistxattr(path, list, size);
    if (res == -1)
        return -errno;
    return res;
}
static int xmp_removexattr(const char *path, const char *name) {
    int res = lremovexattr(path, name);
    if (res == -1)
        return -errno;
    return 0;
}
#endif /* HAVE_SETXATTR */
static off_t xmp_lseek(const char *path, off_t off, int whence, struct fuse_file_info *fi) {
    char fpath[PATH_MAX];
    fullpath2(fpath, source_root, path);

    int fd;
    off_t res;
    if (fi == NULL)
        fd = open(fpath, O_RDONLY);
    else
        fd = fi->fh;
    if (fd == -1)
        return -errno;
    res = lseek(fd, off, whence);
    if (res == -1)
        res = -errno;
    if (fi == NULL)
        close(fd);
    return res;
}
static const struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readlink = xmp_readlink,
    // .mknod = xmp_mknod,
    // .mkdir = xmp_mkdir,
    // .unlink = xmp_unlink,
    // .rmdir = xmp_rmdir,
    // .symlink = xmp_symlink,
    // .rename = xmp_rename,
    // .link = xmp_link,
    // .chmod = xmp_chmod,
    // .chown = xmp_chown,
    // .truncate = xmp_truncate,
    .open = xmp_open,
    .read = xmp_read,
    // .write = xmp_write,
    // .statfs = xmp_statfs,
    // flush
    .release = xmp_release,
// .fsync = xmp_fsync,
#ifdef HAVE_SETXATTR
    .setxattr = xmp_setxattr,
    .getxattr = xmp_getxattr,
    .listxattr = xmp_listxattr,
    .removexattr = xmp_removexattr,
#endif
    //  opendir
    .readdir = xmp_readdir,
    // releasedir
    // fsyncdir
    .init = xmp_init,
    // destroy
    .access = xmp_access,
// .create = xmp_create,
// lock
#ifdef HAVE_UTIMENSAT
    .utimens = xmp_utimens,
#endif
// bmap
// ioctl
// poll
// write_buf
// read_buf
// flock
#ifdef HAVE_POSIX_FALLOCATE
    .fallocate = xmp_fallocate,
#endif
#ifdef HAVE_COPY_FILE_RANGE
    .copy_file_range = xmp_copy_file_range,
#endif
    .lseek = xmp_lseek,
};
int main(int argc, char *argv[]) {

    // {
    //     char fpath[PATH_MAX];
    //     fullpath2(fpath, "/a/", "/b/");
    //     printf("%s\n", fpath);
    // }
    // {
    //     char fpath[PATH_MAX];
    //     fullpath2(fpath, "/a", "b/");
    //     printf("%s\n", fpath);
    // }
    // {
    //     char fpath[PATH_MAX];
    //     fullpath2(fpath, "/a/", "b/");
    //     printf("%s\n", fpath);
    // }
    // {
    //     char fpath[PATH_MAX];
    //     fullpath2(fpath, "/a", "/b/");
    //     printf("%s\n", fpath);
    // }
    // exit(0);

    enum { MAX_ARGS = 10 };
    int new_argc = 0;
    char *new_argv[MAX_ARGS];
    umask(0);
    for (int i = 0; (i < argc) && (new_argc < MAX_ARGS); i++) {
        if (!strcmp(argv[i], "--filter")) {
            filter = path_filter(argv[++i]);
        } else if (!strcmp(argv[i], "--source")) {
            std::strncpy(source_root, argv[++i], PATH_MAX);
        } else {
            new_argv[new_argc++] = argv[i];
        }
    }
    return fuse_main(new_argc, new_argv, &xmp_oper, NULL);
}
