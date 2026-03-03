# Smart Water Bottle - 项目交接文档

> 本文件用于 Claude Code 新会话快速了解项目上下文。将本文件内容作为新对话的开头即可。

---

## 一、项目概述

基于 **STM32F103C8T6** 的智能水杯嵌入式项目，使用 **FreeRTOS** + **HAL** 库，
采用 **BSP + App 三层架构**（Core / BSP / App）。

**核心功能**：水温检测(DS18B20)、水质检测(TDS数字传感器)、水位检测(M04非接触x3)、
OLED显示(SSD1306)、RTC断电计时(LSE+纽扣电池)、杯盖舵机锁(SG90)、
密码解锁(OLED滚动数字+按键确认)、失败报警(蜂鸣器)、蓝牙数据发送(HC-05)、
锂电池充电管理(TP4056状态监测)。

---

## 二、本次会话完成的工作

### 第一次会话（初始开发）

1. **BSP 层** — 14 个头文件 + 13 个源文件
   - bsp_gpio (统一引脚定义)、bsp_ds18b20 (OneWire温度)、bsp_oled (SSD1306软件I2C)
   - bsp_oled_font (6x8 + 8x16 完整ASCII字体)、bsp_tds (数字UART水质)
   - bsp_water_level (M04非接触x3)、bsp_servo (TIM3 PWM)、bsp_buzzer (GPIO)
   - bsp_ir_sensor (GP2Y0A21YK0F红外测距)、bsp_battery (ADC电池电量)
   - bsp_bluetooth (USART2 HC-05)、bsp_key (按键去抖+长按检测)
   - bsp_rtc (LSE+BKP域日期存储)

2. **App 层** — 6 个头文件 + 5 个源文件
   - app_config (共享类型/常量)、app_task (5个FreeRTOS任务创建+实现)
   - app_sensor (互斥锁保护采集)、app_display (OLED多页面显示)
   - app_lock (密码锁状态机+解锁序列)、app_bluetooth (格式化蓝牙发送)

3. **Core 层** — 更新 4 个文件 + 新建 1 个文件
   - main.c (BSP_Init → App_Task_Create → vTaskStartScheduler)
   - main.h (包含 app_config.h)
   - FreeRTOSConfig.h (Cortex-M3, 72MHz, 10KB heap, 1ms tick)
   - stm32f1xx_hal_conf.h (启用 ADC/TIM/UART/RTC/BKP 模块, **USE_RTOS=0**)
   - stm32f1xx_it.c (移除SVC/PendSV, SysTick转发FreeRTOS)

4. **FreeRTOS 内核** — 从 GitHub 官方仓库克隆
   - 放置于 `Middlewares/Third_Party/FreeRTOS/Source/`
   - 包含 RVDS/ARM_CM3 (Keil) 和 GCC/ARM_CM3 两套移植层
   - 使用 heap_4 内存管理方案

5. **文档** — `Doc/README.md` (574行完整文档)
   - 功能说明、引脚分配表、硬件连接图、外设配置、API参考、编译配置、资源估算

### 第二次会话（编译配置修复 + GP2Y0A21YK0F 传感器适配）

1. **修复 HAL USE_RTOS 编译错误**
   - `stm32f1xx_hal_conf.h:138` 将 `USE_RTOS` 从 `1U` 改为 `0U`
   - 原因: STM32 HAL 内部 `USE_RTOS` 宏是预留的未实现功能，必须为 0，与 FreeRTOS 集成无关

2. **修复 FreeRTOS 头文件路径错误**
   - `Smart_Water_Bottle.uvprojx:344` Include Path 添加 `../Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM3`
   - 修复 `portmacro.h` 找不到的编译错误

3. **完善 Keil 工程源文件组**
   - 添加 App 组 (5 个 .c 文件)
   - 添加 BSP 组 (12 个 .c 文件)
   - 添加 Middlewares/FreeRTOS 组 (8 个内核+移植层文件)
   - 添加 Drivers/HAL_Extra 组 (ADC/TIM/UART/RTC HAL 驱动 7 个文件)

4. **修复 RTC 驱动链接错误**
   - `bsp_rtc.c` 新增 `BSP_RTC_ReadCounter()` / `BSP_RTC_WriteCounter()` 直接操作 RTC 寄存器
   - 替换 HAL 内部 `static` 函数 `RTC_ReadTimeCounter()` / `RTC_WriteTimeCounter()` 的调用
   - 使用 double-read 防止高低 16 位读取不一致

5. **GP2Y0A21YK0F 红外测距传感器适配**
   - **问题**: 原代码按数字电平读取，GP2Y0A21YK0F 是模拟输出测距传感器
   - **传感器特性**: 输出 0.4~3.1V 对应 80~10cm，<10cm 非单调但仍高于基准
   - **修复**:
     - `bsp_gpio.h:57-61` 添加 `IR_ADC_CHANNEL` (ADC1_CH7) 和 `IR_CLOSE_THRESHOLD` (1000)
     - `bsp_gpio.c:49-53` PA7 从 `GPIO_MODE_INPUT + PULLUP` 改为 `GPIO_MODE_ANALOG`
     - `bsp_ir_sensor.c` 内部实现 ADC1 初始化和通道读取（独立于 TDS）
     - 新增 `BSP_IR_ReadRaw()` 返回 ADC 原始值（5 次滑动平均）
     - `BSP_IR_IsBlocked()` 通过阈值比较判断杯盖状态
   - **注意**: 阈值需硬件调试时校准（见第七节）

6. **TDS 数字传感器串口驱动改造**（第三次会话：协议修正）
   - **问题**: 原代码按模拟 ADC 读取，实际传感器是带 MCU 的数字型（串口通信）
   - **传感器型号**: 多参数数字传感器（TDS/EC/盐度/比重/温度/硬度，UART 9600bps）
   - **引脚重新分配**:
     - TP4056 CHRG/STDBY 从 PB10/PB11 移动到 **PB12/PB13**（需重新焊接硬件）
     - USART3 (PB10-TX / PB11-RX) 分配给 **TDS 传感器**
     - PA0 原 TDS_ADC_CH0 引脚空闲
   - **驱动改造**:
     - `bsp_tds.h` 新增 `tds_sensor_data_t` 结构体（TDS/EC/盐度/比重/温度/硬度 6个参数）
     - `bsp_tds.c` 完全重写为串口通信（根据实际协议文档修正）：
       - 发送查询命令: `A0 00 00 00 00 00 A0` (7字节)
       - 响应包: `AA [TDS-H TDS-L] [EC-4~EC-1] [SAL-H SAL-L] [SG-H SG-L] [TEM-H TEM-L] [HAR-H HAR-L] [CRC8]` (16字节)
       - CRC8 校验: 前15字节求和
       - 新增 `BSP_TDS_ReadData()` 读取完整数据包
       - 保留 `BSP_TDS_ReadValue()` 向下兼容（仅返回 TDS 值）
     - 新增 `HAL_UART_MspInit()` 回调初始化 USART3 GPIO
   - **注意**: 协议已根据手册修正，硬件测试时需验证字节序和校验算法

7. **M04 非接触水位传感器验证**（第三次会话）
   - **确认**: 实际使用 M04 非接触式电容水位传感器（非光电式）
   - **传感器特性**: 电容式传感器，无需接触液体，开漏输出，有水时输出低电平
   - **验证结果**:
     - GPIO 配置正确：`GPIO_MODE_INPUT + GPIO_PULLUP`（M04 开漏输出必须上拉）✅
     - 读取逻辑正确：低电平表示有水，高电平表示无水 ✅
     - 级联逻辑正确：s1(低)→s1+s2(中)→s1+s2+s3(满) ✅
   - **更新内容**:
     - `bsp_water_level.h/c` 更新注释，添加 M04 特性说明和详细水位检测逻辑
     - `bsp_gpio.h/c` 更新注释，标注"M04 Non-contact Capacitive"并注明开漏特性

8. **电池电量检测功能实现**（第三次会话）
   - **需求变更**: 从 TP4056 充电状态检测改为实时电池电量百分比显示
   - **硬件电路**:
     - 从 TP4056 的 BAT 引脚引出 VBAT
     - 通过分压电路（R1=100kΩ, R2=100kΩ，1:1 分压）连接到 PA0（ADC1_CH0）
     - 电池类型：单节高压锂电池（3.2V~4.35V）
   - **实现方案**:
     - 创建 `bsp_battery.h/c` 驱动模块
     - ADC1 读取 PA0 电压，10 次采样平均滤波
     - 电压计算：`VBAT = (ADC_raw / 4095) * 3.3V * 2`
     - 电量查表法：7 级分段线性插值（4.35V=100% ~ 3.2V=0%）
     - `app_config.h`: 将 `charging_state` 字段改为 `battery_capacity`（0-100%）
     - `app_sensor.c`: 调用 `BSP_Battery_GetCapacity()` 读取电量
     - `app_display.c`: OLED 右下角显示电量百分比（例如"85%"）
   - **注意**: PB12/PB13 的 TP4056 CHRG/STDBY 引脚不再使用（GPIO 配置保留但不读取）

### 第四次会话（OLED 刷新优化 + 串口调试 + 解锁逻辑修复）

1. **OLED 显示刷新优化**（`bsp_oled.c`）
   - **问题**: 屏幕持续逐行刷新，同一时刻无法显示完整界面
   - **根因分析**:
     - `BSP_OLED_Fill(0x00)` 内部调用 `BSP_OLED_Update()`，每个显示周期先推送空白画面再推送内容，造成闪烁
     - `BSP_OLED_Update()` 每次发送全部 8 页（1024 字节），即使内容未变
     - 每个数据字节使用独立 I2C 事务（Start-Addr-Control-Data-Stop），传输极慢
   - **修复**:
     - `BSP_OLED_Clear()` / `BSP_OLED_Fill()` 不再调用 `BSP_OLED_Update()`，仅修改内存缓冲区
     - 新增 `s_gram_shadow` 影子缓冲区（1024 字节），`BSP_OLED_Update()` 逐页 `memcmp` 比较，仅发送有变化的页
     - 每页 128 字节合并为单次 I2C 事务（原来 128 次 Start/Stop 减少为 1 次），传输速度提升约 3-4 倍
     - `BSP_OLED_Init()` 初始化 shadow 为 0xFF，确保首次 Update 发送全部页
   - **效果**: 数据未变时零 I2C 传输；数据变化时仅刷新变化页，无闪烁

2. **串口模拟按键调试功能**（`bsp_debug_uart.c/h`）— 新增模块
   - **需求**: 未安装物理按键时通过串口模拟按键操作
   - **实现**:
     - 使用 USART1 (PA9-TX / PA10-RX, 115200 8N1) 接收中断
     - 发送 ASCII 字符模拟按键事件：`'1'`=KEY1短按、`'2'`=KEY1长按、`'3'`=KEY2短按、`'4'`=KEY2长按
     - `USART1_IRQHandler` 内通过 `xQueueSendFromISR()` 注入事件到 `g_queue_key`，与物理按键共用完全相同的处理路径
     - 中断优先级 6（满足 FreeRTOS `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5` 要求）
     - 收到有效字符回显确认（如 `[K1_SHORT]`），未知字符回显十六进制值（如 `[RX:0x41]`），便于验证接线
     - 上电时通过 polling 发送操作提示信息
   - **接线**: STM32 PA9(TX)→模块RX、PA10(RX)→模块TX、GND→GND
   - **集成**: `bsp.h` 包含头文件，`BSP_Init()` 末尾调用 `BSP_DebugUart_Init()`

3. **解锁后立即重新上锁问题修复**（`app_lock.c`）
   - **问题**: 解锁成功后画面一闪即回到 LOCKED 界面
   - **根因**: 解锁成功后 `g_lock_state=UNLOCKED`，Lock 任务 100ms 后再次执行，`BSP_IR_IsBlocked()` 读取 PA7（未接 IR 传感器时引脚悬空，ADC 随机值大概率超阈值）→ 立刻触发自动上锁
   - **修复**: 新增 `s_unlock_cooldown` 冷却计数器，解锁成功后设为 `UNLOCK_COOLDOWN_CYCLES`（30 × 100ms = 3 秒），冷却期内跳过自动上锁和按键检测

---

## 三、项目文件结构

```
Smart_Water_Bottle/
├── Core/Inc/           main.h, FreeRTOSConfig.h, stm32f1xx_hal_conf.h, stm32f1xx_it.h
├── Core/Src/           main.c, stm32f1xx_it.c, stm32f1xx_hal_msp.c, system_stm32f1xx.c
├── BSP/Inc/            bsp.h, bsp_gpio.h, bsp_ds18b20.h, bsp_oled.h, bsp_oled_font.h,
│                       bsp_tds.h, bsp_water_level.h, bsp_servo.h, bsp_buzzer.h,
│                       bsp_ir_sensor.h, bsp_battery.h, bsp_bluetooth.h, bsp_key.h,
│                       bsp_rtc.h, bsp_debug_uart.h
├── BSP/Src/            对应 14 个 .c 实现文件
├── App/Inc/            app_config.h, app_task.h, app_sensor.h, app_display.h,
│                       app_lock.h, app_bluetooth.h
├── App/Src/            对应 5 个 .c 实现文件
├── Middlewares/Third_Party/FreeRTOS/Source/
│   ├── include/        FreeRTOS 头文件 (22个)
│   ├── *.c             内核源码 (tasks/queue/list/timers/event_groups/stream_buffer)
│   └── portable/       RVDS/ARM_CM3 + GCC/ARM_CM3 + MemMang/heap_4.c
├── Drivers/            STM32 HAL 库 (CubeMX 生成, 未修改)
├── MDK-ARM/            Keil 工程文件
├── Doc/README.md       完整说明文档
└── HANDOFF.md          本交接文档
```

---

## 四、关键引脚分配

| 引脚 | 功能 | 引脚 | 功能 |
|------|------|------|------|
| **PA0** | **VBAT_ADC (ADC1_CH0)** | PA1 | 水位1-低 |
| PA2 | BT TX (USART2) | PA3 | BT RX |
| PA4 | KEY1 解锁键 | PA5 | KEY2 功能键 |
| PA6 | 舵机 (TIM3_CH1) | **PA7** | **GP2Y0A21YK0F (ADC1_CH7)** |
| PA8 | DS18B20 (OneWire) | PA9/10 | 调试串口 (USART1) |
| PB0 | 水位2-中 | PB1 | 水位3-高 |
| PB5 | 蜂鸣器 | PB6 | OLED SCL |
| PB7 | OLED SDA | **PB10** | **TDS TX (USART3)** |
| **PB11** | **TDS RX (USART3)** | **PB12** | **TP4056 CHRG** |
| **PB13** | **TP4056 STDBY** | PC13 | 板载LED |
| PC14/15 | 32.768kHz RTC晶振 | PD0/1 | 8MHz HSE晶振 |

---

## 五、FreeRTOS 任务架构

| 任务 | 优先级 | 栈 | 周期 | 职责 |
|------|--------|-----|------|------|
| Task_Key | 4 | 512B | 20ms | 按键扫描→队列 |
| Task_Lock | 3 | 1KB | 事件 | 锁控状态机+解锁序列 |
| Task_Sensor | 2 | 1KB | 500ms | DS18B20/TDS/水位采集 |
| Task_Display | 2 | 2KB | 200ms | OLED显示刷新 |
| Task_Bluetooth | 1 | 1KB | 2s | 蓝牙数据发送 |

通信: `g_queue_key`(队列) + `g_mutex_sensor`(互斥锁) + `g_system_state/g_lock_state`(volatile原子)

---

## 六、密码锁机制

- 默认密码: `{3, 7, 5}` (定义在 `App/Src/app_lock.c`)
- 流程: 长按KEY1→OLED依次显示1-9→在正确数字时按KEY1确认→3位全对则解锁
- 失败3次→蜂鸣器报警+OLED显示ALARM→长按KEY2重置

---

## 七、待完成 / 后续工作

### 硬件调试必做

1. **TP4056 引脚重新焊接**（PB10/PB11 → PB12/PB13）
   - CHRG 引脚从 PB10 改为 **PB12**
   - STDBY 引脚从 PB11 改为 **PB13**
   - PB10/PB11 腾出给 USART3 (TDS 传感器)

2. **TDS 串口协议硬件验证**（`bsp_tds.c`）
   - 协议已根据官方手册实现（查询命令 `A0 00 00 00 00 00 A0`，响应 16 字节 `AA [DATA] CRC8`）
   - 调试步骤：
     1. 通过串口助手发送 `A0 00 00 00 00 00 A0` 验证传感器响应
     2. 确认响应包头为 `AA`，长度为 16 字节
     3. 验证 CRC8 校验算法（前 15 字节求和）
     4. 对比实测数值与手持检测仪：TDS/EC/盐度/比重/温度/硬度
     5. 如果字节序不匹配（例如传感器使用小端序），修改 `BSP_TDS_ReadData()` 中的解析逻辑
   - **注意**: 当前实现按大端序（高位在前）解析，与手册一致，但需实测验证

3. **GP2Y0A21YK0F 阈值校准**（`bsp_gpio.h:61`）
   - 当前 `IR_CLOSE_THRESHOLD = 1000` 是估算值（对应约 40cm）
   - 实测步骤:
     1. 在 `app_sensor.c` 或串口调试中读取 `BSP_IR_ReadRaw()` 原始值
     2. 记录杯盖完全打开时的 ADC 基准值（通常 <500）
     3. 记录杯盖关闭时的 ADC 峰值（GP2Y0A21YK0F 在 10-40cm 内输出最高）
     4. 阈值设为 `(基准值 + 峰值) / 2`，确保有 200+ 的裕量
   - **注意**: GP2Y0A21YK0F 在 <10cm 时输出非单调（电压下降），但仍高于无物体基准
   - 如果安装距离 <10cm，需通过实测确认阈值有效性

### 可选优化

1. **DS18B20 非阻塞化** — 当前 ReadTemp() 阻塞等待 750ms 转换，可改为分离
   启动转换和读取结果，利用 FreeRTOS vTaskDelay 让出 CPU
2. ~~**OLED 局部刷新**~~ — ✅ 已在第四次会话完成（脏页标记 + 批量 I2C）
3. **蓝牙接收** — 当前只实现了发送，可添加 UART 中断接收 + 命令解析（远程设置时间/密码等）
4. **低功耗** — 添加 FreeRTOS Idle Hook 进入 WFI 低功耗模式
5. **看门狗** — 添加 IWDG 防止系统挂死
6. **OTA/DFU** — 通过蓝牙实现固件更新
7. **多级水质告警** — TDS 阈值超标时蜂鸣器提醒

---

## 八、已知设计决策

| 决策 | 选择 | 原因 |
|------|------|------|
| OLED I2C | 软件 I2C (GPIO) | STM32F1 硬件I2C有已知bug |
| 字体 | 6x8 + 8x16 内嵌数组 | 无需外部文件依赖 |
| RTC 日期 | BKP 寄存器存储 | F1 RTC 只有32位计数器无日历 |
| RTC 读写 | 直接操作寄存器 | HAL `RTC_ReadTimeCounter` 等函数是 `static` |
| 微秒延时 | DWT CYCCNT | 比 SysTick 精确，不占用定时器 |
| 内存管理 | heap_4 | 支持碎片合并，适合动态分配 |
| 密码输入 | 滚动数字+单键确认 | 最少按键实现多位密码 |
| 红外传感器 | GP2Y0A21YK0F (ADC) | 模拟输出测距，PA7-ADC1_CH7 独立采集 |
| 水位传感器 | M04 非接触电容式 | 开漏输出需上拉，低电平表示有水，3级检测 |
| TDS 传感器 | 数字型 USART3 通信 | 多参数传感器（TDS/EC/盐度/比重/温度/硬度） |
| 电池电量检测 | ADC1_CH0 (PA0) | 100kΩ+100kΩ分压，7级查表法，0-100%显示 |
| TP4056 引脚 | PB12/PB13 | 原 PB10/PB11 让给 USART3（CHRG/STDBY不再使用） |
| OLED 刷新 | 脏页+影子缓冲区 | memcmp 逐页比较，未变化页跳过 I2C 传输 |
| OLED I2C 传输 | 每页单次事务批量发送 | 128 字节合并为 1 次 Start/Stop，比逐字节快 3-4 倍 |
| 串口调试 | USART1 中断注入队列 | 与物理按键共用 g_queue_key，零侵入性 |
| 解锁冷却 | 3 秒冷却期 | 防止 IR 悬空时解锁后立即重新上锁 |

---

## 九、快速命令参考

```bash
# 查看项目结构（排除HAL驱动）
find /mnt/d/STM32_Project/Smart_Water_Bottle -maxdepth 4 -type f \( -name "*.c" -o -name "*.h" \) -not -path "*/Drivers/*" | sort

# 统计代码行数
wc -l /mnt/d/STM32_Project/Smart_Water_Bottle/BSP/Src/*.c /mnt/d/STM32_Project/Smart_Water_Bottle/App/Src/*.c

# 查看引脚定义
cat /mnt/d/STM32_Project/Smart_Water_Bottle/BSP/Inc/bsp_gpio.h

# 查看任务配置
cat /mnt/d/STM32_Project/Smart_Water_Bottle/App/Inc/app_config.h

# 查看锁控逻辑
cat /mnt/d/STM32_Project/Smart_Water_Bottle/App/Src/app_lock.c

# 查看 IR 传感器驱动（GP2Y0A21YK0F ADC 采集）
cat /mnt/d/STM32_Project/Smart_Water_Bottle/BSP/Src/bsp_ir_sensor.c

# 查看 ADC 通用通道读取函数
grep -A 15 "BSP_ADC1_ReadChannel" /mnt/d/STM32_Project/Smart_Water_Bottle/BSP/Src/bsp_tds.c
```
