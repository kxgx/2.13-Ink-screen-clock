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

# 启用蓝牙网络共享服务
rfcomm watch hci0 &

# 创建蓝牙网络共享配置文件
cat > /etc/bluetooth/bt-network.conf <<EOF
[General]
Class = 0x000100
Discoverable = True
Pairable = True
EOF

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

# 配置网络接口
nmcli con add type bluetooth ifname $BLUETOOTH_ADAPTER con-name $BLUETOOTH_SERVICE
nmcli con modify $BLUETOOTH_SERVICE bluetooth.type nap
nmcli con modify $BLUETOOTH_SERVICE bluetooth.name $BLUETOOTH_NAP_NAME
nmcli con modify $BLUETOOTH_SERVICE ipv4.method shared
nmcli con modify $BLUETOOTH_SERVICE ipv4.addresses $BLUETOOTH_IP_RANGE
nmcli con modify $BLUETOOTH_SERVICE ipv4.dhcp-range $BLUETOOTH_DHCP_RANGE
nmcli con up $BLUETOOTH_SERVICE

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
