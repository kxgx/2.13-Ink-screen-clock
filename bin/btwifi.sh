#!/bin/bash

# 确保脚本以root用户运行
if [ "$EUID" -ne 0 ]; then
  echo "请以root用户运行或使用sudo"
  exit 1
fi

# 启动蓝牙服务
systemctl start bluetooth

# 使用 bluetoothctl 设置蓝牙
bluetoothctl <<EOF
power on
agent on
default-agent
discoverable on
pairable on
EOF

# 创建rfcomm配置文件
cat > /etc/bluetooth/rfcomm.conf <<EOF
rfcomm0 {
  # Bluetooth adapter for the network sharing
  rfcomm0 /dev/rfcomm0;
  # Bind to the Bluetooth adapter
  bind yes;
  # Set the channel for the RFCOMM protocol
  channel 1;
  # Set the Bluetooth adapter
  device B8:27:EB:BC:77:3A;
  # Comment for the RFCOMM channel
  comment "RFCOMM channel for NAP";
}
EOF

# 启动rfcomm服务
rfcomm listen rfcomm0 &

# 设置网络共享
nmcli connection add type bluetooth ifname bnep0 con-name BluetoothNetwork autoconnect yes
nmcli connection modify BluetoothNetwork bluetooth.bond B8:27:EB:BC:77:3A
nmcli connection modify BluetoothNetwork ipv4.method shared

# 启用IP转发
echo '1' | sudo tee /proc/sys/net/ipv4/ip_forward

# 设置iptables规则以允许流量转发
iptables -t nat -A POSTROUTING -o bnep0 -j MASQUERADE
iptables -A FORWARD -i bnep0 -o bnep0 -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i bnep0 -o bnep0 -j ACCEPT

# 保存iptables规则
iptables-save > /etc/iptables.ipv4.nat

# 将iptables规则加载到rc.local以便在启动时应用
cat >> /etc/rc.local <<EOF

# Load iptables rules
iptables-restore < /etc/iptables.ipv4.nat
EOF

# 使rc.local可执行
chmod +x /etc/rc.local

echo "蓝牙网络共享配置完成 重启后生效"
