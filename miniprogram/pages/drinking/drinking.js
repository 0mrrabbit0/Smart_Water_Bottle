const app = getApp()
const cloudService = require('../../services/cloud')
const { formatDate, getWeekDay } = require('../../utils/util')
const { DRINK_AMOUNTS } = require('../../utils/constants')

Page({
  data: {
    todayStr: '',
    dailyTotal: 0,
    dailyGoal: 2000,
    drinkCount: 0,
    streakDays: 0,
    avgTemp: '--',
    progressPercent: 0,
    quickAmounts: DRINK_AMOUNTS,
    todayRecords: [],
    weekData: [],
    reminderEnabled: false,
    reminderIdx: 1,
    reminderOptions: [
      { label: '每1小时', value: 60 },
      { label: '每2小时', value: 120 },
      { label: '每3小时', value: 180 },
      { label: '每4小时', value: 240 }
    ],
    showGoalModal: false,
    tempGoal: 2000
  },

  onLoad() {
    this.setData({
      todayStr: formatDate() + ' ' + getWeekDay(formatDate()),
      dailyGoal: app.globalData.dailyGoal
    })
    this.loadReminderSetting()
  },

  onShow() {
    this.loadTodayData()
    this.loadWeekData()
  },

  async loadTodayData() {
    try {
      const summary = await cloudService.getDailySummary()
      if (summary) {
        const percent = Math.min(100, Math.round(summary.total_amount / (summary.goal || this.data.dailyGoal) * 100))
        this.setData({
          dailyTotal: summary.total_amount || 0,
          drinkCount: summary.drink_count || 0,
          streakDays: summary.streak_days || 0,
          avgTemp: summary.avg_temperature ? summary.avg_temperature.toFixed(1) + '°C' : '--',
          progressPercent: percent,
          dailyGoal: summary.goal || this.data.dailyGoal
        })
      }

      const records = await cloudService.getTodayRecords()
      const formatted = records.map(r => ({
        ...r,
        timeStr: r.recorded_at ? new Date(r.recorded_at).toTimeString().slice(0, 5) : '--:--'
      }))
      this.setData({ todayRecords: formatted })
    } catch (e) {
      console.error('加载今日数据失败:', e)
    }
  },

  async loadWeekData() {
    try {
      const stats = await cloudService.getWeeklyStats()
      if (stats && stats.length) {
        const dayLabels = ['日', '一', '二', '三', '四', '五', '六']
        const weekData = stats.map(s => {
          const d = new Date(s.date)
          const goal = s.goal || this.data.dailyGoal
          return {
            date: s.date,
            dayLabel: dayLabels[d.getDay()],
            amount: s.amount || 0,
            achieved: s.achieved || false,
            percent: Math.min(100, Math.round((s.amount || 0) / goal * 100))
          }
        })
        this.setData({ weekData })
      }
    } catch (e) {
      console.error('加载周数据失败:', e)
    }
  },

  // 快速记录饮水
  async quickDrink(e) {
    const amount = e.currentTarget.dataset.amount
    wx.showLoading({ title: '记录中...' })
    try {
      await cloudService.addDrinkRecord({
        amount,
        source: 'manual',
        temperature: app.globalData.deviceData.temperature,
        tds: app.globalData.deviceData.tds
      })
      wx.hideLoading()
      wx.showToast({ title: `已记录 ${amount}ml`, icon: 'success' })
      this.loadTodayData()
    } catch (e) {
      wx.hideLoading()
      wx.showToast({ title: '记录失败', icon: 'none' })
    }
  },

  // 自定义饮水量
  customDrink() {
    wx.showModal({
      title: '自定义饮水量',
      editable: true,
      placeholderText: '请输入饮水量(ml)',
      success: async (res) => {
        if (res.confirm && res.content) {
          const amount = parseInt(res.content)
          if (amount > 0 && amount <= 2000) {
            try {
              await cloudService.addDrinkRecord({ amount, source: 'manual' })
              wx.showToast({ title: `已记录 ${amount}ml`, icon: 'success' })
              this.loadTodayData()
            } catch (e) {
              wx.showToast({ title: '记录失败', icon: 'none' })
            }
          } else {
            wx.showToast({ title: '请输入1-2000ml', icon: 'none' })
          }
        }
      }
    })
  },

  // 目标设置
  onGoalTap() {
    this.setData({
      showGoalModal: true,
      tempGoal: this.data.dailyGoal
    })
  },

  onGoalSlide(e) {
    this.setData({ tempGoal: e.detail.value })
  },

  closeGoalModal() {
    this.setData({ showGoalModal: false })
  },

  async confirmGoal() {
    const goal = this.data.tempGoal
    this.setData({
      showGoalModal: false,
      dailyGoal: goal
    })
    app.globalData.dailyGoal = goal
    wx.setStorageSync('dailyGoal', goal)

    try {
      await cloudService.updateGoal(goal)
    } catch (e) {
      console.error('更新目标失败:', e)
    }
    this.loadTodayData()
  },

  // 提醒设置
  loadReminderSetting() {
    const enabled = wx.getStorageSync('reminderEnabled') || false
    const idx = wx.getStorageSync('reminderIdx') || 1
    this.setData({ reminderEnabled: enabled, reminderIdx: idx })
  },

  toggleReminder(e) {
    const enabled = e.detail.value
    this.setData({ reminderEnabled: enabled })
    wx.setStorageSync('reminderEnabled', enabled)
  },

  onReminderChange(e) {
    const idx = parseInt(e.detail.value)
    this.setData({ reminderIdx: idx })
    wx.setStorageSync('reminderIdx', idx)
  },

  // 跳转统计
  goStats() {
    wx.navigateTo({ url: '/pages/stats/stats' })
  },

  onPullDownRefresh() {
    this.loadTodayData()
    this.loadWeekData()
    wx.stopPullDownRefresh()
  }
})
