# 1. 创建ssh目录，防止目录不存在报错
mkdir -p ~/.ssh

# 2. 生成ED25519安全密钥，绑定指定邮箱，无密码免交互
ssh-keygen -t ed25519 -C 1393265226@qq.com -N  -f ~/.ssh/id_ed25519

# 3. 写入SSH核心配置，强制走443端口绕过国内22端口封锁
cat > ~/.ssh/config <<'EOF'
Host github.com
  HostName ssh.github.com
  User git
  Port 443
  IdentityFile ~/.ssh/id_ed25519
  IdentitiesOnly yes
EOF

# 4. 赋予ssh配置文件安全权限（必做，否则SSH拒绝连接）
chmod 700 ~/.ssh
chmod 600 ~/.ssh/config
chmod 600 ~/.ssh/id_ed25519

# 5. 输出公钥，直接复制全部内容
echo ==========下方为你的GitHub公钥，全选复制==========
cat ~/.ssh/id_ed25519.pub
echo ==============================================

git config --global user.name wjlwjlwjlwjl-cmd
git config --global user.email 1393265226@qq.com
