const app = getApp()
const bleManager = require('../../services/ble')

Page({
  data: {
    connected: false,
    scanning: false,
    deviceList: [],
    boundDevice: null,
    intervalIdx: 0,
    intervalOptions: [
      { label: '2秒', value: 2000 },
      { label: '5秒', value: 5000 },
      { label: '10秒', value: 10000 }
    ]
  },

  onLoad() {
    this.setData({ connected: app.globalData.connected })
    this.loadBoundDevice()
  },

  onShow() {
    this.setData({ connected: bleManager.connected })
  },

  loadBoundDevice() {
    const deviceId = wx.getStorageSync('boundDeviceId')
    const deviceName = wx.getStorageSync('boundDeviceName')
    if (deviceId) {
      this.setData({
        boundDevice: { deviceId, name: deviceName || 'SWB-设备' }
      })
    }
  },

  // 开始扫描
  async startScan() {
    if (this.data.scanning) return

    this.setData({ scanning: true, deviceList: [] })

    try {
      const devices = await bleManager.scan(10000)
      this.setData({
        scanning: false,
        deviceList: devices
      })

      if (devices.length === 0) {
        wx.showToast({ title: '未发现设备', icon: 'none' })
      }
    } catch (e) {
      this.setData({ scanning: false })
      wx.showToast({ title: '扫描失败，请检查蓝牙是否开启', icon: 'none' })
    }
  },

  // 选择设备连接
  async selectDevice(e) {
    const device = this.data.deviceList[e.currentTarget.dataset.index]

    wx.showLoading({ title: '连接中...' })
    try {
      await bleManager.connect(device.deviceId)
      wx.hideLoading()

      // 保存绑定信息
      wx.setStorageSync('boundDeviceId', device.deviceId)
      wx.setStorageSync('boundDeviceName', device.name)

      this.setData({
        connected: true,
        boundDevice: device
      })
      app.globalData.connected = true

      wx.showToast({ title: '连接成功', icon: 'success' })

      // 同步时间
      setTimeout(() => bleManager.syncTime(), 1000)
    } catch (e) {
      wx.hideLoading()
      wx.showToast({ title: '连接失败', icon: 'none' })
    }
  },

  // 连接已绑定设备
  async onConnect() {
    if (!this.data.boundDevice) return

    wx.showLoading({ title: '连接中...' })
    try {
      await bleManager.connect(this.data.boundDevice.deviceId)
      wx.hideLoading()
      this.setData({ connected: true })
      app.globalData.connected = true
      wx.showToast({ title: '连接成功', icon: 'success' })
    } catch (e) {
      wx.hideLoading()
      wx.showToast({ title: '连接失败，请重新扫描', icon: 'none' })
    }
  },

  // 断开连接
  async onDisconnect() {
    await bleManager.disconnect()
    this.setData({ connected: false })
    app.globalData.connected = false
    wx.showToast({ title: '已断开', icon: 'success' })
  },

  // 同步时间
  async syncTime() {
    try {
      await bleManager.syncTime()
      wx.showToast({ title: '时间已同步', icon: 'success' })
    } catch (e) {
      wx.showToast({ title: '同步失败', icon: 'none' })
    }
  },

  // 修改密码
  changePassword() {
    wx.showModal({
      title: '修改密码',
      editable: true,
      placeholderText: '输入旧密码,新密码 (如: 375,123)',
      success: async (res) => {
        if (res.confirm && res.content) {
          const parts = res.content.split(',')
          if (parts.length === 2 && parts[0].length === 3 && parts[1].length === 3) {
            try {
              const result = await bleManager.changePassword(parts[0].trim(), parts[1].trim())
              if (result.success) {
                wx.showToast({ title: '密码已修改', icon: 'success' })
              } else {
                wx.showToast({ title: '旧密码错误', icon: 'none' })
              }
            } catch (e) {
              wx.showToast({ title: '修改失败', icon: 'none' })
            }
          } else {
            wx.showToast({ title: '格式: 旧密码,新密码 (各3位)', icon: 'none' })
          }
        }
      }
    })
  },

  // 修改上报间隔
  async onIntervalChange(e) {
    const idx = parseInt(e.detail.value)
    const interval = this.data.intervalOptions[idx]
    this.setData({ intervalIdx: idx })

    if (this.data.connected) {
      try {
        await bleManager.send(`CMD:INTERVAL,MS=${interval.value}\r\n`)
        wx.showToast({ title: '已设置', icon: 'success' })
      } catch (e) {
        wx.showToast({ title: '设置失败', icon: 'none' })
      }
    }
  },

  // 解绑设备
  unbindDevice() {
    wx.showModal({
      title: '解绑设备',
      content: '确定解绑当前设备吗？',
      success: async (res) => {
        if (res.confirm) {
          await bleManager.disconnect()
          wx.removeStorageSync('boundDeviceId')
          wx.removeStorageSync('boundDeviceName')
          this.setData({
            connected: false,
            boundDevice: null
          })
          app.globalData.connected = false
          wx.showToast({ title: '已解绑', icon: 'success' })
        }
      }
    })
  }
})
