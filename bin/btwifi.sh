#!/bin/bash

# 确保脚本以root用户运行
if [ "$EUID" -ne 0 ]; then
  echo "请以root用户运行或使用sudo"
  exit 1
fi

# 蓝牙适配器名称
BLUETOOTH_ADAPTER="hci0"
# 蓝牙网络共享服务名称
BLUETOOTH_SERVICE="nap"
# 蓝牙网络共享服务的名称
BLUETOOTH_NAP_NAME="RaspberryPiNetwork"
# 蓝牙网络共享服务的PIN码
BLUETOOTH_PIN="1234"
# 蓝牙网络共享的IP地址范围
BLUETOOTH_IP_RANGE="192.168.44.1/24"
# 蓝牙网络共享的DHCP范围
BLUETOOTH_DHCP_RANGE="192.168.44.10,192.168.44.50"

# 启动蓝牙服务
systemctl start bluetooth

# 设置蓝牙设备名称
bluetoothctl <<EOF
power on
agent on
default-agent
discoverable on
pairable on
name $BLUETOOTH_NAP_NAME
EOF

# 配置蓝牙网络共享服务
bluetoothctl <<EOF
register $BLUETOOTH_SERVICE
EOF

# 创建rfcomm配置文件
cat > /etc rfcomm.conf <<EOF
rfcomm0 {
  bind yes;
  device $BLUETOOTH_ADAPTER;
  channel 1;
  comment "RFCOMM channel for NAP";
}
EOF

# 启动rfcomm服务
rfcomm bind rfcomm0 &

# 配置dhcpcd
cat > /etc/dhcpcd.conf <<EOF
interface rfcomm0
static ip_address=$BLUETOOTH_IP_RANGE
denyinterfaces $BLUETOOTH_ADAPTER
EOF

# 启动dhcpcd服务
systemctl restart dhcpcd

# 启用IP转发
echo '1' | sudo tee /proc/sys/net/ipv4/ip_forward

# 配置iptables规则以允许流量转发
iptables -t nat -A POSTROUTING -o $BLUETOOTH_ADAPTER -j MASQUERADE
iptables -A FORWARD -i $BLUETOOTH_ADAPTER -o $BLUETOOTH_ADAPTER -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i $BLUETOOTH_ADAPTER -o $BLUETOOTH_ADAPTER -j ACCEPT

# 保存iptables规则
iptables-save > /etc/iptables.ipv4.nat

# 将iptables规则加载到rc.local以便在启动时应用
cat >> /etc/rc.local <<EOF

# Load iptables rules
iptables-restore < /etc/iptables.ipv4.nat
EOF

echo "蓝牙网络共享配置完成。重启后生效"
