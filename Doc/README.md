# Smart Water Bottle - 智能水杯项目

## 项目概述

基于 STM32F103C8T6 主控的智能水杯系统，集成水温检测、水质检测、水位监测、OLED 显示、
蓝牙数据传输、密码锁等功能。系统采用 FreeRTOS 实时操作系统，分层架构设计

---

## 目录结构

```
Smart_Water_Bottle/
├── Core/                           # 核心层 (CubeMX 生成 + 自定义)
│   ├── Inc/
│   │   ├── main.h                  # 主头文件
│   │   ├── FreeRTOSConfig.h        # FreeRTOS 配置
│   │   ├── stm32f1xx_hal_conf.h    # HAL 模块使能配置
│   │   └── stm32f1xx_it.h         # 中断声明
│   └── Src/
│       ├── main.c                  # 主程序入口
│       ├── stm32f1xx_it.c         # 中断服务程序
│       ├── stm32f1xx_hal_msp.c    # HAL MSP 初始化
│       └── system_stm32f1xx.c     # 系统初始化
├── BSP/                            # 板级支持包 (硬件驱动层)
│   ├── Inc/
│   │   ├── bsp.h                   # BSP 统一头文件
│   │   ├── bsp_gpio.h             # GPIO 引脚定义
│   │   ├── bsp_ds18b20.h          # DS18B20 温度传感器
│   │   ├── bsp_oled.h             # SSD1306 OLED 显示驱动
│   │   ├── bsp_oled_font.h        # OLED 字体数据
│   │   ├── bsp_tds.h              # TDS 水质传感器
│   │   ├── bsp_water_level.h      # 光电水位传感器
│   │   ├── bsp_servo.h            # 舵机驱动
│   │   ├── bsp_buzzer.h           # 蜂鸣器驱动
│   │   ├── bsp_ir_sensor.h        # 红外障碍传感器
│   │   ├── bsp_bluetooth.h        # HC-05 蓝牙模块
│   │   ├── bsp_key.h              # 按键驱动
│   │   └── bsp_rtc.h              # RTC 实时时钟
│   └── Src/
│       ├── bsp.c                   # BSP 统一初始化
│       ├── bsp_gpio.c             # GPIO 初始化
│       ├── bsp_ds18b20.c          # DS18B20 驱动实现
│       ├── bsp_oled.c             # OLED 驱动实现
│       ├── bsp_tds.c              # TDS 驱动实现
│       ├── bsp_water_level.c      # 水位驱动实现
│       ├── bsp_servo.c            # 舵机驱动实现
│       ├── bsp_buzzer.c           # 蜂鸣器驱动实现
│       ├── bsp_ir_sensor.c        # 红外传感器驱动实现
│       ├── bsp_bluetooth.c        # 蓝牙驱动实现
│       ├── bsp_key.c              # 按键驱动实现
│       └── bsp_rtc.c              # RTC 驱动实现
├── App/                            # 应用层
│   ├── Inc/
│   │   ├── app_config.h           # 应用配置与共享数据类型
│   │   ├── app_task.h             # FreeRTOS 任务声明
│   │   ├── app_sensor.h           # 传感器应用接口
│   │   ├── app_display.h          # 显示应用接口
│   │   ├── app_lock.h             # 锁控应用接口
│   │   └── app_bluetooth.h       # 蓝牙应用接口
│   └── Src/
│       ├── app_task.c             # 任务创建与任务函数
│       ├── app_sensor.c           # 传感器数据采集
│       ├── app_display.c          # OLED 显示逻辑
│       ├── app_lock.c             # 杯盖锁控逻辑
│       └── app_bluetooth.c       # 蓝牙数据发送
├── Middlewares/
│   └── Third_Party/
│       └── FreeRTOS/              # FreeRTOS 内核源码 (需自行添加)
├── Drivers/                        # STM32 HAL 驱动 (CubeMX 生成)
├── MDK-ARM/                        # Keil MDK 工程文件
├── Doc/                            # 文档
│   └── README.md                  # 本文件
└── Smart_Water_Bottle.ioc         # CubeMX 工程配置
```

---

## 功能说明

### 1. 水温检测
- **传感器**: DS18B20 数字温度传感器
- **协议**: OneWire 单总线协议 (GPIO 软件模拟)
- **精度**: 0.0625°C (12-bit 分辨率)
- **量程**: -55°C ~ +125°C
- **采样周期**: 500ms

### 2. 水质检测 (TDS)
- **传感器**: TDS 模拟传感器
- **接口**: ADC 模拟输入 (12-bit)
- **单位**: ppm (parts per million)
- **公式**: `TDS = (133.42*V^3 - 255.86*V^2 + 857.39*V) * 0.5`
- **采样周期**: 500ms

### 3. 水位检测
- **传感器**: 光电水位传感器 x3 (低/中/高三级)
- **接口**: GPIO 数字输入 (低电平有效)
- **水位等级**:
  - 0 = 空杯 (三个传感器均未检测到水)
  - 1 = 低水位 (仅低位传感器检测到水)
  - 2 = 中水位 (低位+中位传感器检测到水)
  - 3 = 满杯 (三个传感器均检测到水)

### 4. OLED 显示
- **屏幕**: 0.96 寸 SSD1306 128x64 OLED
- **接口**: Software I2C (GPIO 模拟)
- **显示内容**:
  - 第 1 行: 水温 (Temp: XX.X C)
  - 第 2 行: 水质 (TDS: XXXX ppm)
  - 第 3 行: 水位 (Water: XXXX)
  - 第 4 行: 当前时间 (HH:MM:SS)
- **刷新频率**: 5Hz (200ms)

### 5. RTC 实时时钟
- **时钟源**: LSE 32.768kHz 外部晶振
- **备份电源**: CR2032 纽扣电池 (接 VBAT)
- **功能**: 断电后继续计时，上电恢复时间显示
- **日期存储**: 利用 BKP 备份寄存器保存年月日

### 6. 锂电池充电管理
- **充电芯片**: TP4056
- **状态监测**:
  - CHRG 引脚: 低电平 = 正在充电
  - STDBY 引脚: 低电平 = 充电完成
- **显示**: OLED 右上角显示充电状态

### 7. 杯盖锁定机制
- **检测**: 红外障碍传感器检测杯盖关闭状态
- **执行**: SG90 舵机控制杯盖锁扣
- **逻辑**:
  1. 红外传感器检测到杯盖关闭 -> 舵机转至 0° 锁定
  2. 杯盖开启状态 -> 舵机保持 90° 解锁位

### 8. 密码解锁
- **进入**: 长按 KEY1 (>2 秒) 进入解锁模式
- **解锁流程**:
  1. OLED 依次显示数字 1-9 (每个数字显示 1.2 秒)
  2. 当密码对应的数字出现时，按下 KEY1 确认
  3. 默认密码序列: {3, 7, 5} (3 位密码)
  4. 需依次在正确数字出现时按键确认
  5. 全部正确 -> 舵机解锁，杯盖可打开
  6. 按错或超时 -> 失败次数 +1
  7. 连续失败 3 次 -> 进入报警模式
- **报警模式**: 蜂鸣器持续报警 + OLED 显示 "ALARM"
- **报警解除**: 长按 KEY2 重置

### 9. 蓝牙数据传输
- **模块**: HC-05 蓝牙串口模块
- **波特率**: 9600 bps
- **数据格式**: `SWB:T=25.5,TDS=150,WL=3,HH:MM:SS\r\n`
- **发送周期**: 2000ms

---

## FreeRTOS 任务架构

| 任务名 | 优先级 | 栈大小 | 周期 | 功能描述 |
|--------|--------|--------|------|----------|
| Task_Key | 4 (最高) | 128 Words (512B) | 20ms | 按键扫描、去抖、长按检测 |
| Task_Lock | 3 | 256 Words (1KB) | 事件驱动 | 杯盖锁控、解锁序列、报警 |
| Task_Sensor | 2 | 256 Words (1KB) | 500ms | DS18B20/TDS/水位数据采集 |
| Task_Display | 2 | 512 Words (2KB) | 200ms | OLED 显示更新 |
| Task_Bluetooth | 1 (最低) | 256 Words (1KB) | 2000ms | 蓝牙数据发送 |

### 任务间通信

```
Task_Key ──[g_queue_key]──> Task_Lock
                                │
Task_Sensor ──[g_mutex_sensor]──> g_sensor_data <── Task_Display
                                                 <── Task_Bluetooth
                                │
                         g_system_state (原子访问)
                         g_lock_state   (原子访问)
```

- **g_queue_key** (QueueHandle_t): 按键事件队列，深度 5，从 Key 任务发送到 Lock 任务
- **g_mutex_sensor** (SemaphoreHandle_t): 互斥锁，保护 sensor_data_t 共享数据
- **g_system_state**: 系统状态枚举，volatile 原子访问
- **g_lock_state**: 锁定状态枚举，volatile 原子访问

### FreeRTOS 内存配置

| 配置项 | 值 | 说明 |
|--------|-----|------|
| configTOTAL_HEAP_SIZE | 10KB | FreeRTOS 堆内存 |
| configMINIMAL_STACK_SIZE | 128 Words | 最小栈 (Idle 任务) |
| configTICK_RATE_HZ | 1000 | 1ms 节拍 |
| Heap 管理方案 | heap_4 | 支持 malloc/free + 合并碎片 |

---

## 引脚分配表

### STM32F103C8T6 (LQFP48)

| 引脚 | 功能 | 方向 | 模式 | 外设/备注 |
|------|------|------|------|----------|
| PA0 | TDS 传感器 | 模拟输入 | Analog | ADC1_Channel_0 |
| PA1 | 水位传感器 1 (低位) | 数字输入 | GPIO_Input + PullUp | 低电平有效 |
| PA2 | 蓝牙 HC-05 TX | 复用输出 | AF_PP | USART2_TX |
| PA3 | 蓝牙 HC-05 RX | 复用输入 | Input_Floating | USART2_RX |
| PA4 | KEY1 解锁按键 | 数字输入 | GPIO_Input + PullUp | 低电平有效 |
| PA5 | KEY2 功能按键 | 数字输入 | GPIO_Input + PullUp | 低电平有效 |
| PA6 | 舵机 PWM | 复用输出 | AF_PP | TIM3_CH1 (50Hz) |
| PA7 | 红外障碍传感器 | 数字输入 | GPIO_Input + PullUp | 低电平=杯盖关闭 |
| PA8 | DS18B20 数据线 | 双向 | GPIO_Output_OD | OneWire 协议 |
| PA9 | 调试串口 TX | 复用输出 | AF_PP | USART1_TX (115200) |
| PA10 | 调试串口 RX | 复用输入 | Input_Floating | USART1_RX |
| PA13 | SWD 调试数据 | 复用 | AF | SWDIO |
| PA14 | SWD 调试时钟 | 复用 | AF | SWCLK |
| PB0 | 水位传感器 2 (中位) | 数字输入 | GPIO_Input + PullUp | 低电平有效 |
| PB1 | 水位传感器 3 (高位) | 数字输入 | GPIO_Input + PullUp | 低电平有效 |
| PB5 | 蜂鸣器 | 数字输出 | GPIO_Output_PP | 高电平驱动 |
| PB6 | OLED SCL | 开漏输出 | GPIO_Output_OD | Software I2C |
| PB7 | OLED SDA | 开漏输出 | GPIO_Output_OD | Software I2C |
| PB10 | TP4056 CHRG | 数字输入 | GPIO_Input + PullUp | 低电平=充电中 |
| PB11 | TP4056 STDBY | 数字输入 | GPIO_Input + PullUp | 低电平=充满 |
| PC13 | 板载 LED | 数字输出 | GPIO_Output_PP | 低电平点亮 |
| PC14 | RTC 晶振输入 | 振荡器 | OSC32_IN | 32.768kHz |
| PC15 | RTC 晶振输出 | 振荡器 | OSC32_OUT | 32.768kHz |
| PD0 | HSE 晶振输入 | 振荡器 | OSC_IN | 8MHz |
| PD1 | HSE 晶振输出 | 振荡器 | OSC_OUT | 8MHz |

---

## 硬件连接说明

### DS18B20 温度传感器
```
DS18B20         STM32
VCC    ───────  3.3V
GND    ───────  GND
DQ     ───┬───  PA8
           │
          4.7K
           │
          3.3V
```
> 注意: DQ 线需要 4.7K 上拉电阻到 VCC

### SSD1306 OLED (I2C)
```
OLED           STM32
VCC   ───────  3.3V
GND   ───────  GND
SCL   ───────  PB6
SDA   ───────  PB7
```
> I2C 地址: 0x78 (7-bit: 0x3C)
> 使用软件 I2C，PB6/PB7 配置为开漏输出

### TDS 水质传感器
```
TDS Sensor     STM32
VCC   ───────  3.3V (或 5V，视模块而定)
GND   ───────  GND
AOUT  ───────  PA0
```

### 光电水位传感器 (x3)
```
Sensor 1 (低位)  OUT ─── PA1   (安装在杯底附近)
Sensor 2 (中位)  OUT ─── PB0   (安装在杯中部)
Sensor 3 (高位)  OUT ─── PB1   (安装在杯上部)
各传感器 VCC ─── 3.3V, GND ─── GND
```
> 无水时输出高电平，有水时输出低电平

### HC-05 蓝牙模块
```
HC-05          STM32
VCC   ───────  5V (模块自带 3.3V LDO)
GND   ───────  GND
TXD   ───────  PA3 (USART2_RX)
RXD   ───────  PA2 (USART2_TX)
```
> 注意: HC-05 TXD 接 STM32 RX, RXD 接 STM32 TX (交叉连接)
> 默认波特率: 9600 bps

### SG90 舵机
```
SG90           STM32
VCC (红) ────  5V (外部供电，共地)
GND (棕) ────  GND
SIG (橙) ────  PA6 (TIM3_CH1)
```
> PWM 频率: 50Hz, 脉宽 0.5ms~2.5ms 对应 0°~180°

### 红外障碍传感器
```
IR Sensor      STM32
VCC   ───────  3.3V
GND   ───────  GND
OUT   ───────  PA7
```
> 检测到障碍物 (杯盖关闭) 时输出低电平

### TP4056 充电模块
```
TP4056         STM32
CHRG  ───────  PB10  (充电状态, 低电平=充电中)
STDBY ───────  PB11  (待机状态, 低电平=充满)
BAT+  ───────  锂电池正极
BAT-  ───────  锂电池负极/GND
```

### 按键
```
KEY1 ──┤ ├── GND     PA4 (内部上拉)
KEY2 ──┤ ├── GND     PA5 (内部上拉)
```
> 按下时引脚读到低电平

### 蜂鸣器
```
有源蜂鸣器:
PB5 ─── [NPN 三极管 Base (经1K电阻)] ── Collector ── 蜂鸣器- ── VCC
                                        Emitter ──── GND
```
> 或直接使用低功耗有源蜂鸣器连接 PB5 (需确认驱动电流 < 20mA)

### RTC 时钟
```
PC14 ──┤ 32.768kHz ├── PC15
       │  Crystal  │
       └────┬──────┘
            │
           GND (通过负载电容, 2x 6.8pF)

VBAT ────── CR2032 (+) ─── 3V 纽扣电池
GND  ────── CR2032 (-)
```

---

## 外设使用情况

| 外设 | 用途 | 配置 |
|------|------|------|
| ADC1 | TDS 水质传感器 | Channel 0 (PA0), 12-bit, 单次转换 |
| TIM3 | 舵机 PWM | CH1 (PA6), PSC=71, ARR=19999, 50Hz |
| USART1 | 调试串口 | 115200-8-N-1 |
| USART2 | 蓝牙 HC-05 | 9600-8-N-1 |
| RTC | 实时时钟 | LSE 32.768kHz, VBAT 备份 |
| BKP | 日期存储 | DR1=初始化标记, DR2~DR5=日期 |
| GPIO | 多路传感器/控制 | 见引脚分配表 |
| DWT | 微秒延时 | CYCCNT 循环计数器 |
| SysTick | FreeRTOS 节拍 | 1ms (1000Hz) |

---

## API 参考

### BSP 层 API

#### 系统初始化 (bsp.h)
```c
void BSP_Init(void);              // 初始化所有 BSP 模块
void BSP_DelayUs(uint32_t us);    // 微秒级延时 (DWT)
void BSP_DelayMs(uint32_t ms);    // 毫秒级延时
```

#### DS18B20 温度传感器 (bsp_ds18b20.h)
```c
int   BSP_DS18B20_Init(void);     // 初始化, 返回 0=成功
float BSP_DS18B20_ReadTemp(void); // 读取温度, 单位 °C
```

#### OLED 显示 (bsp_oled.h)
```c
void BSP_OLED_Init(void);                                         // 初始化 SSD1306
void BSP_OLED_Clear(void);                                        // 清屏
void BSP_OLED_Update(void);                                       // 刷新显存到屏幕
void BSP_OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t size);
void BSP_OLED_ShowString(uint8_t x, uint8_t y, const char *str, uint8_t size);
void BSP_OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t size);
void BSP_OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode);      // 画点
void BSP_OLED_Fill(uint8_t data);                                  // 填充整屏
```
> size 参数: 12 = 6x8 小字体, 16 = 8x16 大字体

#### TDS 水质传感器 (bsp_tds.h)
```c
void     BSP_TDS_Init(void);       // 初始化 ADC
uint16_t BSP_TDS_ReadRaw(void);    // 读取 ADC 原始值 (0-4095)
uint16_t BSP_TDS_ReadValue(void);  // 读取 TDS 值, 单位 ppm
```

#### 水位传感器 (bsp_water_level.h)
```c
void    BSP_WaterLevel_Init(void);  // 初始化
uint8_t BSP_WaterLevel_Read(void);  // 读取水位等级 (0-3)
```

#### 舵机 (bsp_servo.h)
```c
void BSP_Servo_Init(void);               // 初始化 TIM3 PWM
void BSP_Servo_SetAngle(uint8_t angle);  // 设置角度 (0-180°)
void BSP_Servo_Lock(void);               // 锁定 (0°)
void BSP_Servo_Unlock(void);             // 解锁 (90°)
```

#### 蜂鸣器 (bsp_buzzer.h)
```c
void BSP_Buzzer_Init(void);
void BSP_Buzzer_On(void);
void BSP_Buzzer_Off(void);
void BSP_Buzzer_Toggle(void);
void BSP_Buzzer_Beep(uint16_t on_ms, uint16_t off_ms, uint8_t count);
```

#### 红外障碍传感器 (bsp_ir_sensor.h)
```c
void    BSP_IR_Init(void);
uint8_t BSP_IR_IsBlocked(void);  // 返回 1=检测到障碍 (杯盖关闭)
```

#### 蓝牙 HC-05 (bsp_bluetooth.h)
```c
void BSP_BT_Init(void);
void BSP_BT_SendByte(uint8_t byte);
void BSP_BT_SendString(const char *str);
void BSP_BT_SendData(const uint8_t *data, uint16_t len);
```

#### 按键 (bsp_key.h)
```c
void        BSP_Key_Init(void);
key_event_t BSP_Key_Scan(void);  // 非阻塞扫描, 每 20ms 调用一次
```
> 返回值: KEY_EVENT_NONE / KEY1_SHORT / KEY1_LONG / KEY2_SHORT / KEY2_LONG

#### RTC 时钟 (bsp_rtc.h)
```c
typedef struct { uint8_t hours, minutes, seconds; } rtc_time_t;
typedef struct { uint8_t year, month, day, weekday; } rtc_date_t;

void BSP_RTC_Init(void);
void BSP_RTC_GetTime(rtc_time_t *time);
void BSP_RTC_SetTime(const rtc_time_t *time);
void BSP_RTC_GetDate(rtc_date_t *date);
void BSP_RTC_SetDate(const rtc_date_t *date);
```

### 应用层 API

#### 任务管理 (app_task.h)
```c
void App_Task_Create(void);  // 创建所有 FreeRTOS 任务
```

#### 传感器 (app_sensor.h)
```c
void          App_Sensor_Update(void);   // 更新传感器数据 (互斥保护)
sensor_data_t App_Sensor_GetData(void);  // 获取传感器数据副本
```

#### 显示 (app_display.h)
```c
void App_Display_Normal(void);                          // 正常模式显示
void App_Display_UnlockDigit(uint8_t digit);            // 解锁时显示大数字
void App_Display_UnlockPrompt(uint8_t step, uint8_t total); // 解锁进度
void App_Display_Alarm(void);                           // 报警显示
void App_Display_Locked(void);                          // 锁定状态显示
```

#### 锁控 (app_lock.h)
```c
void         App_Lock_Init(void);
void         App_Lock_Process(key_event_t key_event);  // 锁控状态机
lock_state_t App_Lock_GetState(void);
```

#### 蓝牙 (app_bluetooth.h)
```c
void App_BT_SendStatus(void);  // 发送状态数据
```

---

## 编译配置 (Keil MDK)

### Include 路径
在 Keil MDK 工程 Options -> C/C++ -> Include Paths 中添加:
```
../Core/Inc
../BSP/Inc
../App/Inc
../Middlewares/Third_Party/FreeRTOS/Source/include
../Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM3
```

### 源文件组
在 Keil MDK 工程中添加以下文件组:

| 文件组 | 文件 |
|--------|------|
| BSP | BSP/Src/*.c (所有 12 个文件) |
| App | App/Src/*.c (所有 5 个文件) |
| FreeRTOS | FreeRTOS/Source/tasks.c, queue.c, list.c, timers.c |
| FreeRTOS/Portable | portable/RVDS/ARM_CM3/port.c, portable/MemMang/heap_4.c |

### FreeRTOS 内核获取
FreeRTOS 内核源码需要单独下载并放置到 `Middlewares/Third_Party/FreeRTOS/` 目录:

1. 从 [FreeRTOS 官网](https://www.freertos.org/a00104.html) 下载
2. 或从 STM32CubeF1 固件包中复制 `Middlewares/Third_Party/FreeRTOS/`
3. 所需文件:
   - `Source/include/*.h` (所有头文件)
   - `Source/tasks.c`
   - `Source/queue.c`
   - `Source/list.c`
   - `Source/timers.c`
   - `Source/portable/RVDS/ARM_CM3/port.c` + `portmacro.h`
   - `Source/portable/MemMang/heap_4.c`

### 编译器设置
- **C Standard**: C99
- **Optimization**: -O1 或 -O2
- **预定义宏**: `USE_HAL_DRIVER`, `STM32F103xB`

### 链接器设置
- **Stack Size**: 0x400 (1KB) - 仅用于 main 函数启动前
- **Heap Size**: 0x200 (512B) - 标准库 heap, FreeRTOS 使用自己的 heap

---

## 系统资源估算

### RAM (20KB = 20480 字节)
| 模块 | 估算大小 | 说明 |
|------|----------|------|
| FreeRTOS Heap | 10240B | 任务栈 + 队列 + 互斥锁 |
| OLED 显存 | 1024B | 128x64/8 页面缓冲 |
| 全局变量 | ~512B | 传感器数据、状态、HAL 句柄 |
| 主栈 (MSP) | 1024B | 启动 + 中断 |
| **合计** | **~12800B** | **占用 62.5%** |

### Flash (64KB = 65536 字节)
| 模块 | 估算大小 | 说明 |
|------|----------|------|
| HAL 库 | ~20KB | 已用模块的 HAL 代码 |
| FreeRTOS | ~8KB | 内核 + heap_4 |
| BSP 驱动 | ~8KB | 所有驱动代码 |
| App 逻辑 | ~4KB | 应用代码 |
| 字体数据 | ~2.5KB | 6x8 + 8x16 字体 |
| 中断向量 + 启动 | ~1KB | startup_stm32f103xb.s |
| **合计** | **~43.5KB** | **占用 66.4%** |

---

## 修改密码

解锁密码在 `App/Src/app_lock.c` 中定义:
```c
static const uint8_t s_password[UNLOCK_PASSWORD_LEN] = {3, 7, 5};
```
修改数组中的数字即可更改密码 (数字范围 1-9)。
密码长度由 `App/Inc/app_config.h` 中的 `UNLOCK_PASSWORD_LEN` 控制。

---

## 注意事项

1. **供电**: 舵机 SG90 建议使用 5V 独立供电，与 STM32 共地
2. **DS18B20 上拉**: 数据线必须接 4.7K 上拉电阻
3. **OLED I2C**: 使用软件 I2C，避免 STM32F1 硬件 I2C 已知问题
4. **RTC 晶振**: PC14/PC15 需焊接 32.768kHz 晶振 + 2x 6.8pF 负载电容
5. **FreeRTOS SysTick**: SysTick 同时服务 HAL 和 FreeRTOS
6. **ADC 参考电压**: 默认使用 VDDA (3.3V) 作为 ADC 参考电压
7. **蓝牙配对**: HC-05 模块默认密码通常为 "1234" 或 "0000"
8. **堆栈监控**: 已启用 FreeRTOS 栈溢出检测 (configCHECK_FOR_STACK_OVERFLOW = 2)
