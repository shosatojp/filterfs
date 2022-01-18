# filterfs

filtered overlay filesystem using rsync like filter file.

## Use cases

- filtering on copying from crypted filesystem with reverse mode (gocryptfs, ecryptfs).
- use with tools with no filter.

## Build

```sh
make
```

## Usage

```sh
# create mount point
mkdir mountpoint/

# mount
./filterfs --filter backup.filter --source / mountpoint/

# unmount
fusermount -u mountpoint/
```

`backup.filter` should be like this.

```
- .git
- node_modules
- .ssh

+ /home/
+ /home/**
+ /etc/
+ /etc/**
+ /root/
+ /root/**

- /**
```
