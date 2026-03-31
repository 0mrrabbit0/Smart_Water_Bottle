const app = getApp()
const cloudService = require('../../services/cloud')

Page({
  data: {
    userInfo: {},
    connected: false,
    dailyGoal: 2000,
    streakDays: 0,
    totalDrink: 0,
    achievementCount: 0,
    newAchievements: 0,
    avgGoalRate: 0,
    reminderEnabled: false
  },

  onShow() {
    this.setData({
      connected: app.globalData.connected,
      dailyGoal: app.globalData.dailyGoal
    })
    this.loadUserInfo()
    this.loadStats()
  },

  async loadUserInfo() {
    try {
      const cached = wx.getStorageSync('userInfo')
      if (cached) {
        this.setData({ userInfo: cached })
      }
      const user = await cloudService.getUser()
      if (user) {
        this.setData({ userInfo: user })
        wx.setStorageSync('userInfo', user)
      }
    } catch (e) {
      console.error('加载用户信息失败:', e)
    }
  },

  async loadStats() {
    try {
      const summary = await cloudService.getDailySummary()
      if (summary) {
        this.setData({ streakDays: summary.streak_days || 0 })
      }

      const achievements = await cloudService.getAchievements()
      this.setData({ achievementCount: achievements ? achievements.length : 0 })

      const monthStats = await cloudService.getMonthlyStats()
      if (monthStats) {
        this.setData({
          totalDrink: monthStats.total_amount ? (monthStats.total_amount / 1000).toFixed(1) : 0,
          avgGoalRate: monthStats.goal_rate || 0
        })
      }
    } catch (e) {
      console.error('加载统计数据失败:', e)
    }
  },

  editProfile() {
    wx.getUserProfile({
      desc: '用于展示个人头像和昵称',
      success: async (res) => {
        const info = {
          nickname: res.userInfo.nickName,
          avatar_url: res.userInfo.avatarUrl
        }
        this.setData({ userInfo: { ...this.data.userInfo, ...info } })
        wx.setStorageSync('userInfo', this.data.userInfo)
        try {
          await cloudService.updateUser(info)
        } catch (e) {
          console.error('更新用户信息失败:', e)
        }
      }
    })
  },

  editBodyInfo() {
    wx.showActionSheet({
      itemList: ['设置体重', '设置运动量'],
      success: (res) => {
        if (res.tapIndex === 0) {
          this.setWeight()
        } else {
          this.setExercise()
        }
      }
    })
  },

  setWeight() {
    wx.showModal({
      title: '设置体重',
      editable: true,
      placeholderText: '请输入体重(kg)',
      success: async (res) => {
        if (res.confirm && res.content) {
          const weight = parseFloat(res.content)
          if (weight > 20 && weight < 200) {
            const userInfo = { ...this.data.userInfo, weight }
            this.setData({ userInfo })
            wx.setStorageSync('userInfo', userInfo)
            await cloudService.updateUser({ weight })
          }
        }
      }
    })
  },

  setExercise() {
    wx.showActionSheet({
      itemList: ['低运动量', '中等运动量', '高运动量'],
      success: async (res) => {
        const levels = ['low', 'moderate', 'high']
        const exercise_level = levels[res.tapIndex]
        const userInfo = { ...this.data.userInfo, exercise_level }
        this.setData({ userInfo })
        wx.setStorageSync('userInfo', userInfo)
        await cloudService.updateUser({ exercise_level })
      }
    })
  },

  editGoal() {
    wx.navigateTo({ url: '/pages/drinking/drinking' })
  },

  toggleReminder(e) {
    const enabled = e.detail.value
    this.setData({ reminderEnabled: enabled })
    wx.setStorageSync('reminderEnabled', enabled)
  },

  goDevice() {
    wx.navigateTo({ url: '/pages/device/device' })
  },

  goStats() {
    wx.navigateTo({ url: '/pages/stats/stats' })
  },

  goAchievements() {
    wx.navigateTo({ url: '/pages/achievements/achievements' })
  },

  shareApp() {
    // 触发分享
  },

  onShareAppMessage() {
    return {
      title: '智能水杯 - 科学饮水健康生活',
      path: '/pages/index/index'
    }
  },

  showAbout() {
    wx.showModal({
      title: '关于智能水杯',
      content: '版本: v1.0.0\n基于 STM32 智能水杯配套小程序\n提供饮水管理、水质监测等功能',
      showCancel: false
    })
  }
})
