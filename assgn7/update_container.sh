rm -rf /var/snap/lxd/common/lxd/storage-pools/default/containers/alice1/rootfs/root/secure_chat_app
rm -rf /var/snap/lxd/common/lxd/storage-pools/default/containers/bob1/rootfs/root/secure_chat_app
rm -rf /var/snap/lxd/common/lxd/storage-pools/default/containers/trudy1/rootfs/root/secure_chat_interceptor

cp -r alice1/secure_chat_app /var/snap/lxd/common/lxd/storage-pools/default/containers/alice1/rootfs/root
cp -r bob1/secure_chat_app /var/snap/lxd/common/lxd/storage-pools/default/containers/bob1/rootfs/root
cp -r trudy1/secure_chat_interceptor /var/snap/lxd/common/lxd/storage-pools/default/containers/trudy1/rootfs/root
# cp -r trudy1/* /var/snap/lxd/common/lxd/storage-pools/default/containers/trudy1/rootfs/root