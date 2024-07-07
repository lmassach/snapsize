# snapsize
Utility to measure the size of BTRFS snapshots, taking shared blocks into
account and displaying the shared size appropriately.

This is essentially an utility to compute the disk usage of files, much like
`du`, but it is also able to tell whether two files have any block in common
(which may happen in filesystems that use copy-on-write, or COW, such as
[BTRFS](https://wiki.archlinux.org/title/btrfs)) and to count the repeated
block only once. It makes use of the kernel's
[fiemap ioctl](https://www.kernel.org/doc/Documentation/filesystems/fiemap.txt).
