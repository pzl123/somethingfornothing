#!/bin/bash

################################################################################
# OCPP 2.0.1 固件安全签名与验证全流程演示
# 优化版本：v2.0
# 功能：演示从证书生成到固件验证的完整安全流程
################################################################################

set -e  # 遇到错误立即退出

# 设置颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 配置变量
MANUFACTURER_NAME="deye"
MANUFACTURER_KEY="${MANUFACTURER_NAME}_root.key"
MANUFACTURER_CERT="${MANUFACTURER_NAME}_root.crt"
MANUFACTURER_SRL="${MANUFACTURER_NAME}_root.srl"

FIRMWARE_NAME="firmware"
FIRMWARE_FILE="${FIRMWARE_NAME}.bin"
FIRMWARE_HASH="${FIRMWARE_NAME}.hash.bin"

FIRMWARE_KEY="${FIRMWARE_NAME}_signing.key"
FIRMWARE_CSR="${FIRMWARE_NAME}_signing.csr"
FIRMWAREA_SIGN="${FIRMWARE_NAME}A.sig"
FIRMWAREB_SIGN="${FIRMWARE_NAME}B.sig"
FIRMWARE_CRET="${FIRMWARE_NAME}_signing.crt"

ROOT_CERT_DAYS=3650
SIGNING_CERT_DAYS=1000
REQUEST_ID=12345

# 角色
MANUFACTURER=manufacturer
CHARGE_POINT=charge-point
CENTREAL_SYSTEM=central-system

################################################################################
# 工具函数
################################################################################

print_header() {
    echo -e "\n${YELLOW}=================================================================${NC}"
    echo -e "${YELLOW}  $1${NC}"
    echo -e "${YELLOW}=================================================================${NC}"
}

print_step() {
    echo -e "\n${BLUE}▶ $1${NC}"
}

print_success() {
    echo -e "${GREEN}    $1${NC}"
}

print_error() {
    echo -e "${RED}    $1${NC}"
}

print_info() {
    echo -e "${CYAN}    $1${NC}"
}

print_ocpp_msg() {
    echo -e "${CYAN}   [OCPP] $1${NC}"
}

check_file() {
    if [ ! -f "$1" ]; then
        print_error "文件不存在：$1"
        exit 1
    fi
}

################################################################################
# 主流程
################################################################################

print_header "OCPP 2.0.1 固件安全签名与验证全流程演示"

# 清理旧环境
print_step "清理旧环境..."
rm -rf $MANUFACTURER $CHARGE_POINT $CENTREAL_SYSTEM
mkdir -p $MANUFACTURER $CHARGE_POINT $CENTREAL_SYSTEM

# 准备模拟固件
check_file "$FIRMWARE_FILE"
cp "$FIRMWARE_FILE" ./$MANUFACTURER/
print_success "已创建模拟固件文件：$FIRMWARE_FILE ($(ls -lh $FIRMWARE_FILE | awk '{print $5}'))"

################################################################################
# 第一阶段：制造与预烧录 (Manufacturing)
################################################################################
print_header "第一阶段：生成根证书 (工厂端)"

cd $MANUFACTURER

print_step "生成根私钥 (RSA 4096 位)..."
openssl genrsa -out $MANUFACTURER_KEY 4096 2>/dev/null
print_success "根私钥已生成 ($MANUFACTURER_KEY)"
print_info "此私钥必须严格保密，建议存储在 HSM 中"

print_step "生成自签名根证书..."
openssl req -x509 -new -nodes -key $MANUFACTURER_KEY -sha256 -days $ROOT_CERT_DAYS -out $MANUFACTURER_CERT -subj "/C=CN/ST=Zhejiang/L=Ningbo/O=Deye/OU=Security/CN=Deye ChargePoint Root CA" 2>/dev/null
print_success "根证书已生成 ($MANUFACTURER_CERT)"

# 显示证书信息
print_info "证书信息:"
openssl x509 -in $MANUFACTURER_CERT -noout -subject -dates | sed 's/^/      /'

print_step "生成固件签名、固件证书"

print_step "生成固件签名私钥 (RSA 2048 位)..."
openssl genrsa -out $FIRMWARE_KEY 2048 2>/dev/null
print_success "固件签名私钥($FIRMWARE_KEY)已生成"

print_step "生成证书请求 (CSR)..."
openssl req -new -key $FIRMWARE_KEY -out $FIRMWARE_CSR -subj "/C=CN/ST=Zhejiang/L=Ningbo/O=Deye/OU=Firmware/CN=Deye Firmware Signing Cert" 2>/dev/null
print_success "证书请求($FIRMWARE_CSR)已生成"

print_step "根 CA 签发中间证书... "
openssl x509 -req -in $FIRMWARE_CSR -CA $MANUFACTURER_CERT -CAkey $MANUFACTURER_KEY -CAcreateserial -out $FIRMWARE_CRET -days $SIGNING_CERT_DAYS -sha256 2>/dev/null
print_success "中间证书($FIRMWARE_CRET)已生成"

# 显示证书信息
print_info "证书信息:"
openssl x509 -in $FIRMWARE_CRET -noout -subject -dates | sed 's/^/      /'


################################################################################
# 生成两种格式的签名
################################################################################
print_step "计算固件 SHA256 哈希..."
openssl dgst -SHA256 -binary -out $FIRMWARE_HASH $FIRMWARE_FILE
HASH_VALUE=$(openssl dgst -SHA256 $FIRMWARE_FILE | awk '{print $2}')
print_success "哈希值：$HASH_VALUE"

print_step "生成签名 A (pkeyutl - 原始 RSA 签名)..."
openssl pkeyutl -sign -in "$FIRMWARE_HASH" -inkey "$FIRMWARE_KEY" -out "$FIRMWAREA_SIGN"
print_success "签名 A 已生成 ($FIRMWAREA_SIGN) - $(ls -lh "$FIRMWAREA_SIGN" | awk '{print $5}')"

print_step "生成签名 B (dgst - ASN.1 编码签名)..."
openssl dgst -SHA256 -sign $FIRMWARE_KEY -out $FIRMWAREB_SIGN $FIRMWARE_FILE
print_success "签名 B 已生成 ($FIRMWAREB_SIGN) - $(ls -lh $FIRMWAREB_SIGN | awk '{print $5}')"


cp ./$MANUFACTURER_CERT ../$CHARGE_POINT
print_success "根证书($MANUFACTURER_CERT)已预烧录到充电桩信任库"
cd ..


# ################################################################################
# # 第二阶段：准备固件更新 (Firmware Preparation)
# ################################################################################
print_header "CSMS -> CP : 运营商请求桩 更新固件"

cd $MANUFACTURER
print_info "将固件、固件签名、固件签名证书发送给 ---> CSMS"
cp $FIRMWARE_FILE $FIRMWAREA_SIGN $FIRMWAREB_SIGN $FIRMWARE_CRET ../$CENTREAL_SYSTEM
print_success "已将固件($FIRMWARE_FILE)、固件签名($FIRMWAREA_SIGN $FIRMWAREB_SIGN)、固件签名证书($FIRMWARE_CRET)发送给CSMS"



################################################################################
# 模拟 OCPP 2.0.1 SignedUpdateFirmware.req
################################################################################
print_header "第三阶段：OCPP 消息模拟"

# print_info "$(pwd) $(ls)"
cd ../$CENTREAL_SYSTEM
print_ocpp_msg "CSMS → CP: SignedUpdateFirmware.req(\
[
  2,
  \"$REQUEST_ID\",
  \"UpdateFirmware\",
  {
    \"location\": \"https://ota.deye.com/firmware/v2.0.bin\",
    \"retries\": 3,
    \"retryInterval\": 300,
    \"signature\": \"$(base64 -w0 $FIRMWAREA_SIGN | head -c 50)...)\",
    \"signingCertificate\": \"$(base64 -w0 $FIRMWARE_CRET | head -c 50)...)\"
  }
])"

# cp firmware_signing.crt firmware.sig.A firmware.sig.B ../charge-point/
print_success "升级请求已发送至充电桩"
cp ./$FIRMWAREA_SIGN $FIRMWAREB_SIGN $FIRMWARE_CRET ../$CHARGE_POINT

cd ../$CHARGE_POINT
# ################################################################################
# # 充电桩端验证 (Charge Point Verification)
# ################################################################################
print_step "Verify Certificate"

# 1. 验证证书链
if openssl verify -CAfile $MANUFACTURER_CERT $FIRMWARE_CRET >/dev/null 2>&1; then
    print_success "证书链验证通过"
    openssl x509 -in $FIRMWARE_CRET -noout -issuer | sed 's/^/      /'
else
    print_error "证书链验证失败！拒绝更新"
    exit 1
fi

print_ocpp_msg "CP → CSMS: SignedUpdateFirmware.conf([
  3,
  "2234",
  {
    "status": "Accepted"
  }
])"

print_ocpp_msg "CP → CSMS: SignedFirmwareStatusNotification.req([
  2,
  2321,
  "FirmwareStatusNotification",
  {
  \"status\":\"Downloading\",
   \"requestId\":$REQUEST_ID)
  }
])"
print_ocpp_msg "CSMS -> CP: SignedFirmwareStatusNotification.conf()"
# # 2. 模拟下载固件
print_step "下载固件..."
print_ocpp_msg "CP → CSMS: SignedFirmwareStatusNotification.req (Downloaded)"
print_ocpp_msg "CSMS → CP :  SignedFirmwareStatusNotification.conf()"
cp ../$CENTREAL_SYSTEM/$FIRMWARE_FILE ./
print_success "固件下载完成 ($(ls -lh $FIRMWARE_FILE | awk '{print $5}'))"

print_step "Verify signature"
# # 3. 提取公钥
print_step "从证书提取公钥..."
openssl x509 -in $FIRMWARE_CRET -pubkey -noout -out extracted_pubkey.pem
print_success "公钥已提取 (extracted_pubkey.pem)"

# # 4. 验证签名 - 方法 A
print_step "验证签名 (方法 A: pkeyutl 原始签名)..."
openssl dgst -SHA256 -binary -out local_calculated.hash.bin $FIRMWARE_FILE
if openssl pkeyutl -verify -pubin -inkey extracted_pubkey.pem \
    -in local_calculated.hash.bin -sigfile $FIRMWAREA_SIGN >/dev/null 2>&1; then
    print_success "签名验证通过 (方法 A)"
else
    print_error "签名验证失败 (方法 A)"
fi

# 5. 验证签名 - 方法 B
print_step "验证签名 (方法 B: dgst ASN.1 签名)..."
if openssl dgst -SHA256 -verify extracted_pubkey.pem -signature $FIRMWAREB_SIGN $FIRMWARE_FILE >/dev/null 2>&1; then
    print_success "签名验证通过 (方法 B)"
else
    print_error "签名验证失败 (方法 B)"
fi

print_ocpp_msg "CP → CSMS: SignedFirmwareStatusNotification.req(status = SignatureVerified, requestId = $REQUEST_ID)"
print_ocpp_msg "CSMS → CP: SignedFirmwareStatusNotification.conf()"

################################################################################
# 第五阶段：安装流程模拟
################################################################################
print_header "第五阶段：固件安装流程"

print_step "等待事务完成..."
sleep 1

print_ocpp_msg "CP → CSMS: SignedFirmwareStatusNotification.req(status = InstallRebooting, requestId = $REQUEST_ID)"
print_ocpp_msg "CSMS → CP: SignedFirmwareStatusNotification.conf()"

# 可选择重启
print_ocpp_msg "CSMS → CP: Reset.req (Soft)"
print_ocpp_msg "CP → CSMS: Reset.conf (Accepted)"

print_step "rebooting"
sleep 1

print_ocpp_msg "CP → CSMS: SignedFirmwareStatusNotification.req(status = Installing, requestId = $REQUEST_ID)"
print_ocpp_msg "CSMS → CP: SignedFirmwareStatusNotification.conf()"

print_step "install firmware"
sleep 1
print_ocpp_msg "CP → CSMS: SignedFirmwareStatusNotification.req(status = Installed, requestId = $REQUEST_ID)"
print_ocpp_msg "CSMS → CP: SignedFirmwareStatusNotification.conf()"
print_success "固件安装完成"

# ################################################################################
# # 第六阶段：安全测试 - 模拟篡改
# ################################################################################
# print_header "第六阶段：安全测试 - 模拟固件篡改"

# print_step "创建篡改的固件副本..."
# cp $FIRMWARE_FILE ${FIRMWARE_FILE}.tampered
# echo "HACKED_BY_ATTACKER_$(date +%s)" >> ${FIRMWARE_FILE}.tampered
# print_info "已修改固件内容"

# print_step "验证篡改后的固件..."
# if openssl dgst -SHA256 -verify extracted_pubkey.pem -signature firmware.sig.B ${FIRMWARE_FILE}.tampered >/dev/null 2>&1; then
#     print_error "⚠️  警告：篡改未被检测到！"
# else
#     print_success "安全机制生效：检测到固件被篡改"
#     print_ocpp_msg "CP → CSMS: FirmwareStatusNotification (InstallFailed)"
# fi

# rm -f ${FIRMWARE_FILE}.tampered
# print_info "已清理篡改测试文件"

# ################################################################################
# # 完成
# ################################################################################
# cd ..

# print_header "演示完成"

# echo -e "\n${YELLOW}--- 最终目录结构 ---${NC}"
# echo -e "\n${BLUE}manufacturer/ ${CYAN}(工厂端 - 包含私钥，需严格保护)${NC}"
# ls -1 manufacturer/ | sed 's/^/    /'

# echo -e "\n${BLUE}charge-point/ ${CYAN}(充电桩端 - 只有公钥和固件)${NC}"
# ls -1 charge-point/ | sed 's/^/    /'

# echo -e "\n${YELLOW}--- 安全要点总结 ---${NC}"
# echo -e "${CYAN}  1. 根私钥 (manufacturer_root.key) 必须离线存储${NC}"
# echo -e "${CYAN}  2. 签名私钥 (firmware_signing.key) 建议存储在 HSM 中${NC}"
# echo -e "${CYAN}  3. 充电桩只存储根证书公钥，不存储任何私钥${NC}"
# echo -e "${CYAN}  4. 每次固件更新都需要重新签名${NC}"
# echo -e "${CYAN}  5. 证书有效期建议：根证书 10 年，签名证书 1-3 年${NC}"

# echo -e "\n${GREEN}✅ 所有测试完成！${NC}"