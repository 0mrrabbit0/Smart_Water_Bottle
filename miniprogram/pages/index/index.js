const app = getApp()
const bleManager = require('../../services/ble')
const cloudService = require('../../services/cloud')
const drinkDetector = require('../../services/drink-detector')
const notification = require('../../services/notification')
const { getTemperatureStatus, getBatteryStatus, getWaterLevelText, evaluateParam } = require('../../utils/water-quality')
const { DRINK_AMOUNTS } = require('../../utils/constants')

Page({
  data: {
    connected: false,
    deviceName: '',
    deviceData: {
      temperature: null,
      tds: null,
      waterLevel: 0,
      battery: null,
      time: '--:--:--'
    },
    dailyTotal: 0,
    dailyGoal: 2000,
    tempStatus: { label: '', color: '#8C8C8C' },
    tdsStatus: { label: '', color: '#8C8C8C' },
    batteryStatus: { label: '', color: '#8C8C8C' },
    waterLevelText: '未知',
    drinkAmounts: DRINK_AMOUNTS,
    showDrinkModal: false,
    showUnlockModal: false,
    selectedAmount: 250,
    customAmount: '',
    unlockPassword: ''
  },

  onLoad() {
    // 注册 BLE 数据监听
    this._dataHandler = (data) => this.onDeviceData(data)
    this._statusHandler = (connected) => this.onConnectionChange(connected)
    app.onDeviceData(this._dataHandler)
    app.onConnectionStatus(this._statusHandler)

    // 注册自动饮水检测
    drinkDetector.onDrinkDetected((amount, before, after) => {
      this.onAutoDetectDrink(amount, before, after)
    })

    // 初始化数据
    this.setData({
      connected: app.globalData.connected,
      deviceName: wx.getStorageSync('boundDeviceName') || '',
      dailyGoal: app.globalData.dailyGoal
    })

    // 如果已有缓存数据
    if (app.globalData.deviceData.temperature !== '--') {
      this.updateDeviceDisplay(app.globalData.deviceData)
    }
  },

  onShow() {
    this.loadDailySummary()
  },

  onUnload() {
    app.offDeviceData(this._dataHandler)
    app.offConnectionStatus(this._statusHandler)
  },

  // BLE 数据回调
  onDeviceData(data) {
    this.updateDeviceDisplay(data)
    drinkDetector.feed(data.waterLevel)
    notification.checkBattery(data.battery)
  },

  // 连接状态回调
  onConnectionChange(connected) {
    this.setData({ connected })
  },

  // 更新设备数据显示
  updateDeviceDisplay(data) {
    const updates = { deviceData: data }

    if (data.temperature != null) {
      updates.tempStatus = getTemperatureStatus(data.temperature)
    }
    if (data.tds != null) {
      const tdsResult = evaluateParam(data.tds, 'tds')
      updates.tdsStatus = tdsResult
    }
    if (data.battery != null) {
      updates.batteryStatus = getBatteryStatus(data.battery)
    }
    if (data.waterLevel != null) {
      updates.waterLevelText = getWaterLevelText(data.waterLevel)
    }

    this.setData(updates)
  },

  // 加载今日汇总
  async loadDailySummary() {
    try {
      const summary = await cloudService.getDailySummary()
      if (summary) {
        this.setData({
          dailyTotal: summary.total_amount || 0,
          dailyGoal: summary.goal || app.globalData.dailyGoal
        })
        app.globalData.dailyTotal = summary.total_amount || 0
      }
    } catch (e) {
      console.error('加载每日汇总失败:', e)
    }
  },

  // 点击蓝牙状态
  onBleStatusTap() {
    if (this.data.connected) {
      wx.showActionSheet({
        itemList: ['断开连接', '设备详情'],
        success: (res) => {
          if (res.tapIndex === 0) {
            bleManager.disconnect()
            this.setData({ connected: false })
          } else if (res.tapIndex === 1) {
            wx.navigateTo({ url: '/pages/device/device' })
          }
        }
      })
    } else {
      wx.navigateTo({ url: '/pages/device/device' })
    }
  },

  // 记录饮水
  onDrinkTap() {
    this.setData({
      showDrinkModal: true,
      selectedAmount: 250,
      customAmount: ''
    })
  },

  selectAmount(e) {
    this.setData({
      selectedAmount: e.currentTarget.dataset.amount,
      customAmount: ''
    })
  },

  onCustomInput(e) {
    this.setData({
      customAmount: e.detail.value,
      selectedAmount: 0
    })
  },

  closeDrinkModal() {
    this.setData({ showDrinkModal: false })
  },

  async confirmDrink() {
    const amount = this.data.customAmount
      ? parseInt(this.data.customAmount)
      : this.data.selectedAmount

    if (!amount || amount <= 0 || amount > 2000) {
      wx.showToast({ title: '请输入有效的饮水量', icon: 'none' })
      return
    }

    this.setData({ showDrinkModal: false })

    try {
      wx.showLoading({ title: '记录中...' })
      await cloudService.addDrinkRecord({
        amount,
        source: 'manual',
        temperature: this.data.deviceData.temperature,
        tds: this.data.deviceData.tds
      })
      wx.hideLoading()

      const newTotal = this.data.dailyTotal + amount
      this.setData({ dailyTotal: newTotal })
      app.globalData.dailyTotal = newTotal

      wx.showToast({ title: `已记录 ${amount}ml`, icon: 'success' })
    } catch (e) {
      wx.hideLoading()
      wx.showToast({ title: '记录失败', icon: 'none' })
    }
  },

  // 自动检测饮水
  onAutoDetectDrink(amount, levelBefore, levelAfter) {
    wx.showModal({
      title: '检测到饮水',
      content: `检测到饮水约 ${amount}ml，是否记录？`,
      success: async (res) => {
        if (res.confirm) {
          try {
            await cloudService.addDrinkRecord({
              amount,
              source: 'auto',
              temperature: this.data.deviceData.temperature,
              tds: this.data.deviceData.tds,
              waterLevelBefore: levelBefore,
              waterLevelAfter: levelAfter
            })
            const newTotal = this.data.dailyTotal + amount
            this.setData({ dailyTotal: newTotal })
            app.globalData.dailyTotal = newTotal
            wx.showToast({ title: `已记录 ${amount}ml`, icon: 'success' })
          } catch (e) {
            wx.showToast({ title: '记录失败', icon: 'none' })
          }
        }
      }
    })
  },

  // 远程开锁
  onUnlockTap() {
    if (!this.data.connected) {
      wx.showToast({ title: '请先连接设备', icon: 'none' })
      return
    }
    this.setData({ showUnlockModal: true, unlockPassword: '' })
  },

  onPasswordInput(e) {
    this.setData({ unlockPassword: e.detail.value })
  },

  closeUnlockModal() {
    this.setData({ showUnlockModal: false })
  },

  async confirmUnlock() {
    const pwd = this.data.unlockPassword
    if (!pwd || pwd.length !== 3) {
      wx.showToast({ title: '请输入3位密码', icon: 'none' })
      return
    }

    this.setData({ showUnlockModal: false })

    try {
      wx.showLoading({ title: '开锁中...' })
      const result = await bleManager.unlock(pwd)
      wx.hideLoading()

      if (result.success) {
        wx.showToast({ title: '开锁成功', icon: 'success' })
      } else {
        wx.showToast({ title: '密码错误', icon: 'none' })
      }
    } catch (e) {
      wx.hideLoading()
      wx.showToast({ title: '开锁失败', icon: 'none' })
    }
  },

  // 下拉刷新
  async onPullDownRefresh() {
    await this.loadDailySummary()
    if (this.data.connected) {
      bleManager.query()
    }
    wx.stopPullDownRefresh()
  }
})
