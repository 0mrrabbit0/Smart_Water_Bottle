const bleManager = require('./services/ble')
const cloudService = require('./services/cloud')

App({
  globalData: {
    userInfo: null,
    deviceData: {
      temperature: '--',
      tds: '--',
      waterLevel: 0,
      battery: 0,
      time: '--:--:--'
    },
    connected: false,
    dailyTotal: 0,
    dailyGoal: 2000
  },

  onLaunch() {
    if (!wx.cloud) {
      console.error('请使用 2.2.3 以上的基础库以使用云能力')
      return
    }
    wx.cloud.init({
      traceUser: true
    })

    this.loadCachedData()
    this.initBLE()
  },

  loadCachedData() {
    try {
      const cached = wx.getStorageSync('lastDeviceData')
      if (cached) {
        this.globalData.deviceData = cached
      }
      const goal = wx.getStorageSync('dailyGoal')
      if (goal) {
        this.globalData.dailyGoal = goal
      }
      const userInfo = wx.getStorageSync('userInfo')
      if (userInfo) {
        this.globalData.userInfo = userInfo
      }
    } catch (e) {
      console.error('加载缓存数据失败:', e)
    }
  },

  initBLE() {
    bleManager.onData((data) => {
      this.globalData.deviceData = data
      this.globalData.connected = true
      wx.setStorageSync('lastDeviceData', data)

      // 通知所有监听页面
      if (this._dataListeners) {
        this._dataListeners.forEach(fn => fn(data))
      }
    })

    bleManager.onDisconnect(() => {
      this.globalData.connected = false
      if (this._statusListeners) {
        this._statusListeners.forEach(fn => fn(false))
      }
      // 自动重连
      this.autoReconnect()
    })
  },

  autoReconnect() {
    const deviceId = wx.getStorageSync('boundDeviceId')
    if (!deviceId) return

    let retryCount = 0
    const maxRetry = 3
    const delays = [2000, 4000, 8000]

    const tryConnect = () => {
      if (retryCount >= maxRetry) {
        console.log('自动重连失败，已达最大重试次数')
        return
      }
      setTimeout(async () => {
        try {
          await bleManager.connect(deviceId)
          this.globalData.connected = true
          if (this._statusListeners) {
            this._statusListeners.forEach(fn => fn(true))
          }
        } catch (e) {
          retryCount++
          tryConnect()
        }
      }, delays[retryCount] || 8000)
    }
    tryConnect()
  },

  // 页面注册数据监听
  onDeviceData(callback) {
    if (!this._dataListeners) this._dataListeners = []
    this._dataListeners.push(callback)
  },

  offDeviceData(callback) {
    if (this._dataListeners) {
      this._dataListeners = this._dataListeners.filter(fn => fn !== callback)
    }
  },

  onConnectionStatus(callback) {
    if (!this._statusListeners) this._statusListeners = []
    this._statusListeners.push(callback)
  },

  offConnectionStatus(callback) {
    if (this._statusListeners) {
      this._statusListeners = this._statusListeners.filter(fn => fn !== callback)
    }
  }
})
