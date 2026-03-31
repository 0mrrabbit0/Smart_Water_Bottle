const app = getApp()
const cloudService = require('../../services/cloud')
const { getWeekDay } = require('../../utils/util')

Page({
  data: {
    activeTab: 'week',
    dailyGoal: 2000,
    totalAmount: 0,
    avgAmount: 0,
    achievedDays: 0,
    totalDays: 7,
    goalRate: 0,
    chartData: [],
    calendarDays: [],
    monthLabel: '',
    weekDays: ['日', '一', '二', '三', '四', '五', '六'],
    detailList: [],
    currentYear: 0,
    currentMonth: 0
  },

  onLoad() {
    const now = new Date()
    this.setData({
      dailyGoal: app.globalData.dailyGoal,
      currentYear: now.getFullYear(),
      currentMonth: now.getMonth() + 1
    })
    this.loadWeekStats()
  },

  switchTab(e) {
    const tab = e.currentTarget.dataset.tab
    this.setData({ activeTab: tab })
    if (tab === 'week') {
      this.loadWeekStats()
    } else {
      this.loadMonthStats()
    }
  },

  async loadWeekStats() {
    try {
      const stats = await cloudService.getWeeklyStats()
      if (!stats || !stats.length) {
        this.setData({
          chartData: [],
          totalAmount: 0,
          avgAmount: 0,
          achievedDays: 0,
          goalRate: 0,
          detailList: []
        })
        return
      }

      const goal = this.data.dailyGoal
      const maxAmount = Math.max(...stats.map(s => s.amount || 0), goal)

      const chartData = stats.map(s => ({
        date: s.date,
        label: getWeekDay(s.date).replace('周', ''),
        amount: s.amount || 0,
        achieved: (s.amount || 0) >= goal,
        percent: Math.round(((s.amount || 0) / maxAmount) * 100)
      }))

      const totalAmount = stats.reduce((sum, s) => sum + (s.amount || 0), 0)
      const achievedDays = chartData.filter(d => d.achieved).length

      const detailList = stats.map(s => ({
        date: s.date,
        dateLabel: s.date.slice(5),
        weekday: getWeekDay(s.date),
        amount: s.amount || 0,
        achieved: (s.amount || 0) >= goal,
        percent: Math.min(100, Math.round(((s.amount || 0) / goal) * 100))
      })).reverse()

      this.setData({
        chartData,
        totalAmount,
        avgAmount: Math.round(totalAmount / stats.length),
        achievedDays,
        totalDays: stats.length,
        goalRate: Math.round(achievedDays / stats.length * 100),
        detailList
      })
    } catch (e) {
      console.error('加载周统计失败:', e)
    }
  },

  async loadMonthStats() {
    try {
      const { currentYear, currentMonth } = this.data
      this.setData({ monthLabel: `${currentYear}年${currentMonth}月` })

      const month = `${currentYear}-${String(currentMonth).padStart(2, '0')}`
      const stats = await cloudService.getMonthlyStats(month)

      if (!stats || !stats.days) {
        this.setData({
          calendarDays: [],
          totalAmount: 0,
          avgAmount: 0,
          achievedDays: 0,
          goalRate: 0,
          detailList: []
        })
        return
      }

      const goal = this.data.dailyGoal
      const daysInMonth = new Date(currentYear, currentMonth, 0).getDate()
      const firstDayOfWeek = new Date(currentYear, currentMonth - 1, 1).getDay()

      // 构建日历
      const calendarDays = []
      // 填充前置空白
      for (let i = 0; i < firstDayOfWeek; i++) {
        calendarDays.push({ empty: true })
      }
      // 填充日期
      for (let d = 1; d <= daysInMonth; d++) {
        const dateKey = `${month}-${String(d).padStart(2, '0')}`
        const dayData = stats.days.find(s => s.date === dateKey)
        const amount = dayData ? dayData.amount : 0
        const ratio = amount / goal
        let color = '#F0F0F0'
        if (ratio > 1) color = '#0050B3'
        else if (ratio > 0.75) color = '#1890FF'
        else if (ratio > 0.5) color = '#69C0FF'
        else if (ratio > 0.25) color = '#BAE7FF'
        else if (ratio > 0) color = '#E6F7FF'

        calendarDays.push({ day: d, amount, color })
      }

      const totalAmount = stats.total_amount || 0
      const activeDays = (stats.days || []).filter(d => d.amount > 0).length
      const achievedDays = (stats.days || []).filter(d => (d.amount || 0) >= goal).length

      const detailList = (stats.days || []).map(s => ({
        date: s.date,
        dateLabel: s.date.slice(5),
        weekday: getWeekDay(s.date),
        amount: s.amount || 0,
        achieved: (s.amount || 0) >= goal,
        percent: Math.min(100, Math.round(((s.amount || 0) / goal) * 100))
      })).reverse()

      this.setData({
        calendarDays,
        totalAmount,
        avgAmount: activeDays > 0 ? Math.round(totalAmount / activeDays) : 0,
        achievedDays,
        totalDays: activeDays,
        goalRate: activeDays > 0 ? Math.round(achievedDays / activeDays * 100) : 0,
        detailList
      })
    } catch (e) {
      console.error('加载月统计失败:', e)
    }
  },

  prevMonth() {
    let { currentYear, currentMonth } = this.data
    currentMonth--
    if (currentMonth < 1) {
      currentMonth = 12
      currentYear--
    }
    this.setData({ currentYear, currentMonth })
    this.loadMonthStats()
  },

  nextMonth() {
    let { currentYear, currentMonth } = this.data
    const now = new Date()
    if (currentYear === now.getFullYear() && currentMonth === now.getMonth() + 1) return

    currentMonth++
    if (currentMonth > 12) {
      currentMonth = 1
      currentYear++
    }
    this.setData({ currentYear, currentMonth })
    this.loadMonthStats()
  }
})
