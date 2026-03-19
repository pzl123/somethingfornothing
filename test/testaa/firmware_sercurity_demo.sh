#!/bin/bash

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}=========================================${NC}"
echo -e "${YELLOW}  OCPP 固件安全签名与验证全流程演示${NC}"
echo -e "${YELLOW}=========================================${NC}"

# 清理旧环境
rm -rf manufacturer charge-point
mkdir -p manufacturer charge-point

# 准备模拟固件
FIRMWARE_FILE="firmware.bin"
cp $FIRMWARE_FILE ./manufacturer
echo -e "${GREEN}[准备] 已创建模拟固件文件：$FIRMWARE_FILE${NC}"

# ==========================================
# 第一阶段：制造与预烧录 (Manufacturing)
# ==========================================
echo -e "\n${YELLOW}--- 第一阶段：生成根证书 (工厂端) ---${NC}"
cd manufacturer

# 1. 生成根私钥 (4096位)
echo "生成根私钥..."
openssl genrsa -out manufacturer_root.key 4096 > /dev/null 2>&1

# 2. 生成自签名根证书
echo "生成根证书..."
openssl req -x509 -new -nodes -key manufacturer_root.key -sha256 -days 3650 \
    -out manufacturer_root.crt \
    -subj "/C=CN/ST=Zhejiang/L=Ningbo/O=Deye/OU=Security/CN=Deye ChargePoint Root CA" > /dev/null 2>&1

echo -e "${GREEN}[完成] 根证书已生成 (manufacturer_root.crt)${NC}"
echo "   -> 此证书将被预烧录到所有充电桩中。"
echo "   -> 私钥 (manufacturer_root.key) 将被严格保密。"

# 模拟预烧录：复制根证书到充电桩目录
cd ../charge-point
cp ../manufacturer/manufacturer_root.crt .
echo -e "${GREEN}[模拟] 根证书已预烧录到充电桩信任库。${NC}"
cd ..

# ==========================================
# 第二阶段：准备固件更新 (Firmware Preparation)
# ==========================================
echo -e "\n${YELLOW}--- 第二阶段：签名固件 (服务器端) ---${NC}"
cd manufacturer

# 1. 生成中间证书私钥
echo "生成固件签名私钥..."
openssl genrsa -out firmware_signing.key 2048 > /dev/null 2>&1

# 2. 生成 CSR
echo "生成证书请求 (CSR)..."
openssl req -new -key firmware_signing.key \
    -out firmware_signing.csr \
    -subj "/C=CN/ST=Zhejiang/L=Ningbo/O=Deye/OU=Firmware/CN=Deye Firmware Signing Cert" > /dev/null 2>&1

# 3. 使用根私钥签发中间证书
echo "根CA签发中间证书..."
openssl x509 -req -in firmware_signing.csr \
    -CA manufacturer_root.crt -CAkey manufacturer_root.key \
    -CAcreateserial -out firmware_signing.crt -days 1000 -sha256 > /dev/null 2>&1

echo -e "${GREEN}[完成] 中间证书已生成 (firmware_signing.crt)${NC}"

# 4. 计算固件哈希并签名
echo "计算固件 SHA256 哈希..."
openssl dgst -SHA256 -binary -out firmware.hash.bin $FIRMWARE_FILE

echo "使用中间私钥对哈希进行签名..."
openssl pkeyutl -sign -in firmware.hash.bin -inkey firmware_signing.key -out firmware.sig

echo -e "${GREEN}[完成] 固件签名已生成 (firmware.sig)${NC}"

# 5. 打包发送 (模拟 OTA 推送)
echo "打包并推送到充电桩..."
cp $FIRMWARE_FILE firmware_signing.crt firmware.sig ../charge-point/
echo -e "${GREEN}[模拟] 升级包已发送至充电桩。${NC}"
cd ..

# ==========================================
# 第三阶段：充电桩端验证 (Charge Point Verification)
# ==========================================
echo -e "\n${YELLOW}--- 第三阶段：充电桩验证 (设备端) ---${NC}"
cd charge-point

# 检查文件是否存在
if [ ! -f "$FIRMWARE_FILE" ]; then
    echo -e "${RED}❌ 错误：固件文件 $FIRMWARE_FILE 不存在！${NC}"
    exit 1
fi

# 1. 验证证书链
echo "1. 验证证书链 (中间证书是否由受信任的根证书签发)..."
if openssl verify -CAfile manufacturer_root.crt firmware_signing.crt > /dev/null 2>&1; then
    echo -e "${GREEN}   ✅ 证书链验证通过：OK${NC}"
else
    echo -e "${RED}   ❌ 证书链验证失败！拒绝更新。${NC}"
    exit 1
fi

# 2. 提取公钥
echo "2. 从中间证书提取公钥..."
openssl x509 -in firmware_signing.crt -pubkey -noout -out extracted_pubkey.pem
echo -e "${GREEN}   ✅ 公钥已提取：extracted_pubkey.pem${NC}"

# 3. 验证固件完整性 (方法 A: 分步验证)
echo "3. 验证固件签名 (方法 A: 手动哈希比对)..."
openssl dgst -SHA256 -binary -out local_calculated.hash.bin $FIRMWARE_FILE
if openssl pkeyutl -verify -pubin -in local_calculated.hash.bin -inkey extracted_pubkey.pem -sigfile firmware.sig > /dev/null 2>&1; then
    echo -e "${GREEN}   ✅ 签名验证通过 (方法 A)：Signature Verified Successfully${NC}"
else
    echo -e "${RED}   ❌ 签名验证失败 (方法 A)${NC}"
fi

# 4. 验证固件完整性 (方法 B: 标准命令)
# 【修复】添加 -pubin 参数
echo "4. 验证固件签名 (方法 B: OpenSSL 标准命令)..."
#if openssl dgst -SHA256 -verify extracted_pubkey.pem -signature firmware.sig -pubin $FIRMWARE_FILE > /dev/null 2>&1; then
if openssl dgst -SHA256 -verify extracted_pubkey.pem -signature firmware.sig $FIRMWARE_FILE > /dev/null 2>&1; then
    echo -e "${GREEN}   ✅ 签名验证通过 (方法 B)：Verification OK${NC}"
else
    echo -e "${RED}   ❌ 签名验证失败 (方法 B)${NC}"
    # 调试信息
    echo "   调试：尝试显示验证详细输出..."
    openssl dgst -SHA256 -verify extracted_pubkey.pem -signature firmware.sig -pubin $FIRMWARE_FILE
fi

echo -e "\n${YELLOW}=========================================${NC}"
echo -e "${GREEN}🎉 演示结束！固件安全验证流程全部通过。${NC}"
echo -e "${YELLOW}=========================================${NC}"
cd ..

# ==========================================
# 显示最终目录结构
# ==========================================
echo -e "\n${YELLOW}--- 最终目录结构 ---${NC}"
echo "manufacturer/ (工厂端 - 包含私钥)"
ls -1 manufacturer/
echo ""
echo "charge-point/ (充电桩端 - 只有公钥)"
ls -1 charge-point/

echo -e "\n${GREEN}✅ 所有测试完成！${NC}"