# Smart Water Bottle

基于 STM32F103C8T6 + FreeRTOS 的智能水杯系统，集成温度检测、水质分析、水位监测、密码锁、蓝牙数据传输等功能。

## 硬件平台

| 项目 | 规格 |
|------|------|
| MCU | STM32F103C8T6 (Cortex-M3, 64KB Flash, 20KB RAM) |
| 时钟 | HSI 8MHz x PLL16 / 2 = 64MHz |
| 操作系统 | FreeRTOS (heap_4, 12KB heap) |
| 工具链 | Keil MDK-ARM v5 |

## 功能特性

- **温度检测** - DS18B20 单总线温度传感器 (PA8)，精度 0.0625 C
- **水质分析** - TDS 多参数传感器 (USART3)，输出 TDS/EC/盐度/比重/硬度
- **水位监测** - M04 非接触式电容传感器，2 级检测 (PB0 低水位, PB1 高水位)
- **OLED 显示** - SSD1306 128x64 软件 I2C (PB6 SCL, PB7 SDA)，shadow buffer + dirty page 优化
- **密码锁** - SG90 舵机 (PA6 TIM3)，3 位滚动密码，3 次错误触发警报
- **红外检测** - GP2Y0A21YK0F 距离传感器 (PA1 ADC_CH1)，自动检测杯盖关闭
- **蓝牙传输** - HC-05 (USART2 9600bps)，每 2s 发送 `SWB:T=xx,TDS=xx,WL=x,HH:MM:SS`
- **电池管理** - ADC 电压检测 (PA0)，100K+100K 分压，7 级查表插值
- **实时时钟** - RTC + LSE 32.768kHz，BKP 寄存器存储日期，VBAT 掉电保持
- **蜂鸣器** - 警报/提示音 (PB5)
- **按键** - KEY1 (PA4) 解锁操作, KEY2 (PA5) 警报复位，支持短按/长按

## 项目结构

```
Smart_Water_Bottle/
├── Core/                   # HAL 系统层
│   ├── Inc/                #   FreeRTOSConfig.h, main.h, stm32f1xx_it.h
│   └── Src/                #   main.c (启动入口), stm32f1xx_it.c (中断)
├── BSP/                    # 板级支持包 (硬件驱动层)
│   ├── Inc/                #   bsp_gpio.h (引脚定义), bsp_*.h
│   └── Src/                #   14 个驱动模块
│       ├── bsp.c           #     统一初始化 + 共享 ADC1
│       ├── bsp_oled.c      #     SSD1306 I2C 驱动 (shadow buffer)
│       ├── bsp_ds18b20.c   #     OneWire 温度协议
│       ├── bsp_tds.c       #     USART3 水质查询/应答
│       ├── bsp_water_level.c #   2 级水位 GPIO 读取
│       ├── bsp_ir_sensor.c #     ADC 滑动平均滤波
│       ├── bsp_battery.c   #     ADC 查表电量计算
│       ├── bsp_servo.c     #     TIM3 PWM 舵机控制
│       ├── bsp_buzzer.c    #     蜂鸣器开关
│       ├── bsp_key.c       #     按键消抖 + 长按检测
│       ├── bsp_bluetooth.c #     HC-05 串口发送
│       ├── bsp_rtc.c       #     RTC 日历管理
│       └── bsp_gpio.c      #     GPIO 统一初始化
├── App/                    # 应用逻辑层
│   ├── Inc/                #   app_config.h (共享类型/常量)
│   └── Src/
│       ├── app_task.c      #     FreeRTOS 任务创建 + 主循环
│       ├── app_sensor.c    #     传感器数据采集 (互斥量保护)
│       ├── app_display.c   #     OLED 页面渲染
│       ├── app_lock.c      #     密码锁状态机
│       └── app_bluetooth.c #     蓝牙数据打包发送
├── Middlewares/            # FreeRTOS 内核
├── Drivers/                # STM32 HAL 库
├── MDK-ARM/                # Keil 工程文件
├── Doc/                    # 文档
│   ├── README.md           #   详细技术文档
│   └── system_flowchart.drawio  # 系统架构流程图
└── Smart_Water_Bottle.ioc  # STM32CubeMX 配置
```

## 系统架构

系统采用三层架构: **Core (HAL)** → **BSP (驱动)** → **App (业务逻辑)**

### 启动流程

```
main() → HAL_Init() → SystemClock_Config(64MHz)
       → BSP_Init() (14 个硬件模块)
       → App_Task_Create() (Mutex + Queue + 5 个任务)
       → vTaskStartScheduler()
```

### FreeRTOS 任务

| 任务 | 优先级 | 栈 (字) | 周期 | 功能 |
|------|--------|---------|------|------|
| Task_Key | 4 | 128 | 20ms | 按键扫描消抖，事件入队 |
| Task_Lock | 3 | 384 | 事件驱动 + 100ms | 锁状态机：自动锁定/密码解锁/警报 |
| Task_Sensor | 2 | 384 | 500ms | 采集温度/水质/水位/电量 |
| Task_Display | 2 | 512 | 200ms | OLED 5Hz 刷新，脏页跳过 |
| Task_Bluetooth | 1 | 384 | 2000ms | 蓝牙数据包发送 |

### 任务间通信

- `g_queue_key` - FreeRTOS 队列 (深度 5)，Key → Lock 传递按键事件
- `g_mutex_sensor` - 互斥量保护 `sensor_data_t` 共享数据
- `g_system_state` / `g_lock_state` - volatile 原子状态变量

### 密码锁状态机

```
UNLOCKED ──(IR检测杯盖关闭)──→ LOCKED
    ↑                             │
    │ 密码正确                KEY1长按
    │                             ↓
    └──────── UNLOCK_MODE ────→ ALARM
              3位滚动密码       (失败>=3次)
              KEY1确认               │
                                KEY2长按复位
                                     ↓
                                  LOCKED
```

## 引脚分配

| 功能 | 引脚 | 模式 | 备注 |
|------|------|------|------|
| Battery ADC | PA0 | Analog | ADC1_CH0, 100K+100K 分压 |
| IR Sensor | PA1 | Analog | ADC1_CH1, GP2Y0A21YK0F |
| BT TX/RX | PA2/PA3 | USART2 | HC-05, 9600bps |
| KEY1/KEY2 | PA4/PA5 | Input PU | 低电平有效 |
| Servo | PA6 | TIM3_CH1 | SG90, 50Hz PWM |
| DS18B20 | PA8 | Open-Drain | 单总线，需 4.7K 上拉 |
| Debug TX/RX | PA9/PA10 | USART1 | 115200bps |
| Water Level Low/High | PB0/PB1 | Input PU | M04 电容式，低电平=有水 |
| Buzzer | PB5 | Push-Pull | 高电平驱动 |
| OLED SCL/SDA | PB6/PB7 | Open-Drain PU | 软件 I2C, ~100kHz |
| TDS TX/RX | PB10/PB11 | USART3 | 9600bps |
| TP4056 CHRG/STDBY | PB12/PB13 | Input PU | 充电状态检测 |
| LED | PC13 | Push-Pull | 板载 LED (低电平点亮) |

## 编译与烧录

1. 使用 Keil MDK-ARM v5 打开 `MDK-ARM/Smart_Water_Bottle.uvprojx`
2. 编译: Project → Build Target (F7)
3. 烧录: Flash → Download (F8)，使用 ST-Link 或 J-Link

## 设计要点

- **共享 ADC1**: 电池和红外传感器共用 ADC1 外设，通过 `BSP_ADC1_ReadChannel()` 切换通道读取
- **软件 I2C**: STM32F1 硬件 I2C 存在已知问题，使用 GPIO 模拟 I2C 驱动 OLED
- **OLED 优化**: shadow buffer 对比 + 脏页跳过 + 批量 I2C 传输 (每页一次地址头 + 128 字节数据)
- **DWT 微秒延时**: 使用 Cortex-M3 DWT CYCCNT 实现精确微秒级延时
- **掉电保持**: RTC 使用 VBAT 供电，日期存储在 BKP 备份寄存器中

## 流程图

系统架构流程图位于 [Doc/system_flowchart.drawio](Doc/system_flowchart.drawio)，可用 [draw.io](https://app.diagrams.net/) 打开查看。
