# Define target IP and MAC addresses
target_ip="172.31.0.3"
target_mac="00:16:3e:3d:17:94"

# Construct the command to send ARP packet
nping --dest-ip $target_ip --arp --arp-type arp-reply --source-ip $target_ip --source-mac $target_mac -c 1