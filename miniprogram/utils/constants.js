// BLE 通信常量
const BLE = {
  SERVICE_UUID: '0000FFE0-0000-1000-8000-00805F9B34FB',
  RX_CHAR_UUID: '0000FFE1-0000-1000-8000-00805F9B34FB',
  TX_CHAR_UUID: '0000FFE2-0000-1000-8000-00805F9B34FB',
  DEVICE_NAME_PREFIX: 'SWB-',
  SCAN_TIMEOUT: 10000,
  RECONNECT_MAX_RETRY: 3,
  RECONNECT_DELAYS: [2000, 4000, 8000]
}

// 水位映射容量 (ml)
const WATER_LEVEL_CAPACITY = {
  3: 150,  // Level 3→2
  2: 150,  // Level 2→1
  1: 100   // Level 1→0
}

// 水质等级标准
const QUALITY_STANDARDS = {
  tds: [
    { max: 100, grade: 'A', label: '优质' },
    { max: 300, grade: 'B', label: '良好' },
    { max: 600, grade: 'C', label: '一般' },
    { max: Infinity, grade: 'D', label: '较差' }
  ],
  ec: [
    { max: 200, grade: 'A', label: '优质' },
    { max: 600, grade: 'B', label: '良好' },
    { max: 1200, grade: 'C', label: '一般' },
    { max: Infinity, grade: 'D', label: '较差' }
  ],
  hardness: [
    { max: 60, grade: 'A', label: '优质' },
    { max: 120, grade: 'B', label: '良好' },
    { max: 180, grade: 'C', label: '一般' },
    { max: Infinity, grade: 'D', label: '较差' }
  ],
  salinity: [
    { max: 0.05, grade: 'A', label: '优质' },
    { max: 0.1, grade: 'B', label: '良好' },
    { max: 0.2, grade: 'C', label: '一般' },
    { max: Infinity, grade: 'D', label: '较差' }
  ]
}

// 等级颜色
const GRADE_COLORS = {
  A: '#52C41A',
  B: '#1890FF',
  C: '#FAAD14',
  D: '#F5222D'
}

// 饮水量快捷选项
const DRINK_AMOUNTS = [150, 250, 350, 500]

// 默认每日目标
const DEFAULT_DAILY_GOAL = 2000

// 成就定义
const ACHIEVEMENTS = [
  { id: 'first_drink', name: '初次饮水', desc: '首次记录饮水', icon: '/static/images/ach_first.png' },
  { id: 'streak_7', name: '连续7天', desc: '连续7天达标', icon: '/static/images/ach_streak7.png' },
  { id: 'streak_30', name: '连续30天', desc: '连续30天达标', icon: '/static/images/ach_streak30.png' },
  { id: 'quality_100', name: '水质达人', desc: '累计检测水质100次', icon: '/static/images/ach_quality.png' },
  { id: 'drink_1000', name: '千杯纪念', desc: '累计饮水1000杯', icon: '/static/images/ach_1000.png' },
  { id: 'health_star', name: '健康之星', desc: '月均达标率>90%', icon: '/static/images/ach_star.png' }
]

module.exports = {
  BLE,
  WATER_LEVEL_CAPACITY,
  QUALITY_STANDARDS,
  GRADE_COLORS,
  DRINK_AMOUNTS,
  DEFAULT_DAILY_GOAL,
  ACHIEVEMENTS
}
