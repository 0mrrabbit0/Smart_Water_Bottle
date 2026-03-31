/**
 * 格式化日期为 YYYY-MM-DD
 */
function formatDate(date) {
  const d = date || new Date()
  const year = d.getFullYear()
  const month = String(d.getMonth() + 1).padStart(2, '0')
  const day = String(d.getDate()).padStart(2, '0')
  return `${year}-${month}-${day}`
}

/**
 * 格式化时间为 HH:MM
 */
function formatTime(date) {
  const d = date || new Date()
  const hours = String(d.getHours()).padStart(2, '0')
  const minutes = String(d.getMinutes()).padStart(2, '0')
  return `${hours}:${minutes}`
}

/**
 * 格式化完整日期时间
 */
function formatDateTime(date) {
  return `${formatDate(date)} ${formatTime(date)}`
}

/**
 * 获取最近 N 天的日期数组
 */
function getRecentDays(n) {
  const days = []
  for (let i = n - 1; i >= 0; i--) {
    const d = new Date()
    d.setDate(d.getDate() - i)
    days.push(formatDate(d))
  }
  return days
}

/**
 * 获取星期几
 */
function getWeekDay(dateStr) {
  const weekDays = ['日', '一', '二', '三', '四', '五', '六']
  const d = new Date(dateStr)
  return '周' + weekDays[d.getDay()]
}

/**
 * 计算推荐饮水量
 */
function calcRecommendedGoal(weight, exerciseLevel, month) {
  let base = weight * 30
  const exerciseMap = { low: 0, moderate: 150, high: 300 }
  base += exerciseMap[exerciseLevel] || 0
  // 季节修正 (6-8月夏季, 12-2月冬季)
  if (month >= 6 && month <= 8) {
    base += 300
  } else if (month === 12 || month <= 2) {
    base -= 200
  }
  return Math.round(base / 50) * 50 // 取整到50ml
}

/**
 * 防抖函数
 */
function debounce(fn, delay) {
  let timer = null
  return function (...args) {
    if (timer) clearTimeout(timer)
    timer = setTimeout(() => fn.apply(this, args), delay)
  }
}

/**
 * 节流函数
 */
function throttle(fn, interval) {
  let lastTime = 0
  return function (...args) {
    const now = Date.now()
    if (now - lastTime >= interval) {
      lastTime = now
      fn.apply(this, args)
    }
  }
}

module.exports = {
  formatDate,
  formatTime,
  formatDateTime,
  getRecentDays,
  getWeekDay,
  calcRecommendedGoal,
  debounce,
  throttle
}
