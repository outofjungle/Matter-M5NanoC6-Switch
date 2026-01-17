# Production Security Configuration Checklist

**WARNING: This device currently uses DEVELOPMENT/TEST configuration unsuitable for production deployment.**

Before deploying to production, complete ALL items in this checklist. Failure to properly configure security settings will result in vulnerable devices that can be compromised.

## Critical Security Items

### [ ] 1. Device Attestation Certificates (DAC)

**Current State:** Using example DAC provider (INSECURE)
```
CONFIG_EXAMPLE_DAC_PROVIDER=y
```

**Required for Production:**
```diff
-CONFIG_SEC_CERT_DAC_PROVIDER=y
-CONFIG_EXAMPLE_DAC_PROVIDER=y
+CONFIG_FACTORY_PARTITION_DAC_PROVIDER=y
```

**Action Required:**
1. Obtain official vendor ID from CSA (Connectivity Standards Alliance)
2. Generate production DAC certificates signed by your Product Attestation Authority (PAA)
3. Provision certificates in factory partition during manufacturing
4. Test commissioning with production certificates

**Security Impact:** Example DAC provider uses publicly known test certificates. Any device using these can be impersonated.

---

### [ ] 2. Vendor and Product IDs

**Current State:** Using test IDs
```
CONFIG_DEVICE_VENDOR_ID=0xFFF1  # ESP-Matter test vendor
CONFIG_DEVICE_PRODUCT_ID=0x8000 # Generic test product
```

**Required for Production:**
```
CONFIG_DEVICE_VENDOR_ID=0xYOUR_VID  # Your official CSA vendor ID
CONFIG_DEVICE_PRODUCT_ID=0xYOUR_PID # Your registered product ID
```

**Action Required:**
1. Register with CSA and obtain official Vendor ID
2. Register your product ID for this device
3. Update sdkconfig.defaults with your IDs
4. Update CHIPProjectConfig.h if vendor name is customized

**Security Impact:** Test vendor IDs (0xFFF1, 0xFFF2) are publicly known and explicitly for testing only. Using them in production violates Matter specification.

---

### [ ] 3. Secure Boot and Flash Encryption

**Current State:** NOT ENABLED (flash contents readable, firmware unsigned)

**Required for Production:**
```
CONFIG_SECURE_BOOT_V2_ENABLED=y
CONFIG_SECURE_FLASH_ENC_ENABLED=y
CONFIG_SECURE_FLASH_REQUIRE_ALREADY_ENABLED=y
CONFIG_SECURE_BOOT_INSECURE_ALLOW_JTAG=n
CONFIG_SECURE_BOOT_INSECURE_ALLOW_ROM_BASIC=n
```

**Action Required:**
1. Enable Secure Boot V2 with your signing key
2. Enable Flash Encryption
3. Burn eFuses in production (ONE-TIME operation)
4. Test firmware updates with OTA
5. Document key management procedures

**Security Impact:**
- Without secure boot: Attackers can load malicious firmware
- Without flash encryption: Device secrets (private keys, credentials) readable from flash

**CRITICAL:** Once eFuses are burned, the device cannot be reverted. Test thoroughly on development devices first.

---

### [ ] 4. Debug Interfaces

**Current State:** Debug features enabled

**Matter Shell (saves ~30KB RAM):**
```diff
-CONFIG_ENABLE_CHIP_SHELL=y
+# CONFIG_ENABLE_CHIP_SHELL is not set
```

**OpenThread CLI:**
```
# Already disabled - GOOD
CONFIG_OPENTHREAD_CLI=n
```

**JTAG Access:**
```
# Disable JTAG in production (via Secure Boot config)
CONFIG_SECURE_BOOT_INSECURE_ALLOW_JTAG=n
```

**Action Required:**
1. Disable Matter Shell in sdkconfig.defaults
2. Verify OpenThread CLI remains disabled
3. Disable JTAG via secure boot configuration
4. Remove any debug logging of sensitive data

**Security Impact:** Debug interfaces expose device internals and can be used to extract credentials or modify device behavior.

---

### [ ] 5. Logging Level

**Current State:** INFO level (verbose, may leak sensitive data)
```
CONFIG_LOG_DEFAULT_LEVEL_INFO=y
```

**Required for Production:**
```diff
-CONFIG_LOG_DEFAULT_LEVEL_INFO=y
+CONFIG_LOG_DEFAULT_LEVEL_WARN=y
```

**Action Required:**
1. Change default log level to WARN
2. Audit all log statements for sensitive data:
   - Private keys, certificates
   - Pairing codes, setup PINs
   - User data, network credentials
3. Remove or protect sensitive logging

**Security Impact:** Verbose logging may expose credentials, network topology, or other sensitive information through serial console or log files.

---

### [ ] 6. BLE Configuration

**Current State:** BLE enabled continuously
```
CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING=n
```

**Recommended for Production:**
```diff
-CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING=n
+CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING=y
```

**Action Required:**
1. Enable BLE only for commissioning
2. Test commissioning flow with BLE auto-disable
3. Verify Thread networking continues after BLE disabled

**Security Impact:** Leaving BLE enabled after commissioning increases attack surface. BLE should only be active during initial setup.

---

## Additional Hardening (Recommended)

### [ ] 7. Factory Reset Protection

**Current:** 5-second button hold initiates factory reset

**Recommendation:**
- Require physical access confirmation
- Log factory reset events
- Rate-limit factory reset attempts
- Consider requiring commissioner re-authentication

**Implementation:** Review app_reset.cpp for additional safeguards

---

### [ ] 8. Network Security

**Thread Network:**
- Use unique Thread network credentials (don't use defaults)
- Rotate Thread network key periodically
- Restrict Thread Border Router access
- Monitor for rogue border routers

**Matter Network:**
- Use strong fabric credentials
- Implement node operational certificate rotation
- Monitor for unusual commissioning attempts

---

### [ ] 9. Physical Security

**Consider:**
- Tamper detection (if applicable to hardware)
- Secure enclosure to prevent hardware access
- Anti-rollback protection for firmware
- Secure factory provisioning process

---

### [ ] 10. OTA Update Security

**Current State:** OTA enabled
```
CONFIG_ENABLE_OTA_REQUESTOR=y
```

**Required for Production:**
1. Sign all OTA firmware images
2. Implement rollback protection
3. Verify update source authentication
4. Test OTA with secure boot enabled
5. Implement update size and version checks

**Security Impact:** Unsigned OTA updates can be used to install malicious firmware.

---

## Pre-Production Testing

Before deployment, verify:

- [ ] Device commissions successfully with production DAC
- [ ] Secure boot prevents unsigned firmware loading
- [ ] Flash encryption prevents credential extraction
- [ ] Debug interfaces are disabled and inaccessible
- [ ] BLE disables after commissioning
- [ ] OTA updates work with production configuration
- [ ] Factory reset requires appropriate authorization
- [ ] No sensitive data in production logs
- [ ] Network connectivity stable with production settings
- [ ] Memory and performance acceptable with security features

---

## Compliance

Ensure compliance with:
- [ ] Matter specification security requirements
- [ ] CSA certification requirements
- [ ] Regional regulatory requirements (FCC, CE, etc.)
- [ ] Privacy regulations (GDPR, CCPA, etc.)
- [ ] Industry-specific standards (if applicable)

---

## Key Management

Document and implement:
- [ ] DAC certificate storage and provisioning process
- [ ] Secure boot key generation and storage
- [ ] Flash encryption key management
- [ ] Key rotation procedures
- [ ] Key backup and recovery procedures
- [ ] Key revocation process

---

## Incident Response

Prepare for security incidents:
- [ ] Vulnerability disclosure process
- [ ] Security update/patch distribution plan
- [ ] Device recall procedure (if needed)
- [ ] User notification process
- [ ] Security monitoring and logging

---

## References

- Matter Specification: Security requirements (Chapter 6)
- ESP-IDF Security Guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/security/
- ESP Secure Boot V2: https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/security/secure-boot-v2.html
- ESP Flash Encryption: https://docs.espressif.com/projects/esp-idf/en/latest/esp32c6/security/flash-encryption.html
- CSA Vendor Registration: https://csa-iot.org/become-member/

---

**FINAL WARNING:** Do not deploy devices to production without completing this entire checklist. Security vulnerabilities in IoT devices can compromise user privacy, network security, and brand reputation.

**Personal Project Note:** While this is a personal project, following these guidelines is essential practice for professional IoT development and protects your own network and data.
