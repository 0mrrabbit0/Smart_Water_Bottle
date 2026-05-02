const { QUALITY_STANDARDS, GRADE_COLORS } = require('./constants')

/**
 * 评估单项水质参数等级
 */
function evaluateParam(value, paramType) {
  const standards = QUALITY_STANDARDS[paramType]
  if (!standards) return { grade: '-', label: '未知', color: '#8C8C8C' }

  for (const s of standards) {
    if (value <= s.max) {
      return {
        grade: s.grade,
        label: s.label,
        color: GRADE_COLORS[s.grade]
      }
    }
  }
  return { grade: 'D', label: '较差', color: GRADE_COLORS.D }
}

/**
 * 综合水质评估 (取最差单项)
 */
function evaluateOverall(data) {
  const gradeOrder = ['A', 'B', 'C', 'D']
  let worstIdx = 0

  const params = [
    { type: 'tds', value: data.tds },
    { type: 'ec', value: data.ec },
    { type: 'hardness', value: data.hardness },
    { type: 'salinity', value: data.salinity }
  ]

  params.forEach(p => {
    if (p.value == null) return
    const result = evaluateParam(p.value, p.type)
    const idx = gradeOrder.indexOf(result.grade)
    if (idx > worstIdx) worstIdx = idx
  })

  const grade = gradeOrder[worstIdx]
  const labels = { A: '优质', B: '良好', C: '一般', D: '较差' }
  return {
    grade,
    label: labels[grade],
    color: GRADE_COLORS[grade]
  }
}

/**
 * 获取水温状态
 */
function getTemperatureStatus(temp) {
  if (temp < 10) return { label: '冰水', color: '#1890FF' }
  if (temp < 30) return { label: '常温', color: '#52C41A' }
  if (temp < 50) return { label: '温水', color: '#FAAD14' }
  if (temp < 70) return { label: '热水', color: '#FA8C16' }
  return { label: '烫!注意', color: '#F5222D' }
}

/**
 * 获取电量状态
 */
function getBatteryStatus(level) {
  if (level > 60) return { label: '充足', color: '#52C41A' }
  if (level > 20) return { label: '中等', color: '#FAAD14' }
  return { label: '低电量', color: '#F5222D' }
}

/**
 * 水位等级转文字
 */
function getWaterLevelText(level) {
  const texts = ['空杯', '低', '中', '满']
  return texts[level] || '未知'
}

/**
 * 计算 TDS 进度条百分比 (0-1000 映射到 0-100%)
 */
function getTdsPercent(tds) {
  return Math.min(100, Math.round(tds / 10))
}

module.exports = {
  evaluateParam,
  evaluateOverall,
  getTemperatureStatus,
  getBatteryStatus,
  getWaterLevelText,
  getTdsPercent
}
