alice_target_ip="172.31.0.2"
alice_target_mac="00:16:3e:3d:17:94"
# MAC address of host M
m_mac="00:11:22:33:44:55"

# IP address of host B
b_ip="192.168.1.100"

# Construct the ARP gratuitous packet using hping3
hping3 -2 -c 1 -a $alice_target_mac -k -p 0 -E "00:00:00:00:00:00" -i u1 $alice_target_ip