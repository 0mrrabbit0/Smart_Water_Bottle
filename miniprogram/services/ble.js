const { BLE } = require('../utils/constants')

class BLEManager {
  constructor() {
    this.deviceId = ''
    this.serviceId = BLE.SERVICE_UUID
    this.rxCharId = BLE.RX_CHAR_UUID
    this.txCharId = BLE.TX_CHAR_UUID
    this.connected = false
    this.dataBuffer = ''
    this._dataListeners = []
    this._disconnectListeners = []
    this._responseResolve = null
  }

  /**
   * 扫描附近 BLE 设备
   */
  async scan(timeout) {
    timeout = timeout || BLE.SCAN_TIMEOUT
    await wx.openBluetoothAdapter()
    return new Promise((resolve) => {
      const devices = []
      const seen = new Set()

      wx.onBluetoothDeviceFound((res) => {
        res.devices.forEach(d => {
          if (d.name && d.name.startsWith(BLE.DEVICE_NAME_PREFIX) && !seen.has(d.deviceId)) {
            seen.add(d.deviceId)
            devices.push({
              deviceId: d.deviceId,
              name: d.name,
              RSSI: d.RSSI
            })
          }
        })
      })

      wx.startBluetoothDevicesDiscovery({
        allowDuplicatesKey: false,
        fail: () => resolve([])
      })

      setTimeout(() => {
        wx.stopBluetoothDevicesDiscovery()
        resolve(devices)
      }, timeout)
    })
  }

  /**
   * 连接设备
   */
  async connect(deviceId) {
    this.deviceId = deviceId

    await wx.createBLEConnection({ deviceId })

    // 监听断开事件
    wx.onBLEConnectionStateChange((res) => {
      if (res.deviceId === this.deviceId && !res.connected) {
        this.connected = false
        this._disconnectListeners.forEach(fn => fn())
      }
    })

    // 获取服务
    await wx.getBLEDeviceServices({ deviceId })

    // 获取特征值
    await wx.getBLEDeviceCharacteristics({
      deviceId,
      serviceId: this.serviceId
    })

    // 订阅通知
    await wx.notifyBLECharacteristicValueChange({
      deviceId,
      serviceId: this.serviceId,
      characteristicId: this.rxCharId,
      state: true
    })

    // 监听数据
    wx.onBLECharacteristicValueChange((res) => {
      if (res.deviceId === this.deviceId) {
        this._onData(res.value)
      }
    })

    this.connected = true
  }

  /**
   * 发送指令到设备
   */
  async send(command) {
    if (!this.connected) {
      throw new Error('设备未连接')
    }

    const buffer = new ArrayBuffer(command.length)
    const view = new Uint8Array(buffer)
    for (let i = 0; i < command.length; i++) {
      view[i] = command.charCodeAt(i)
    }

    // BLE 单次最大 20 字节, 需要分包
    const chunkSize = 20
    for (let offset = 0; offset < view.length; offset += chunkSize) {
      const end = Math.min(offset + chunkSize, view.length)
      const chunk = buffer.slice(offset, end)
      await wx.writeBLECharacteristicValue({
        deviceId: this.deviceId,
        serviceId: this.serviceId,
        characteristicId: this.txCharId,
        value: chunk
      })
    }
  }

  /**
   * 发送指令并等待响应
   */
  sendAndWait(command, timeout) {
    timeout = timeout || 5000
    return new Promise((resolve, reject) => {
      this._responseResolve = resolve

      setTimeout(() => {
        if (this._responseResolve === resolve) {
          this._responseResolve = null
          reject(new Error('指令响应超时'))
        }
      }, timeout)

      this.send(command).catch(reject)
    })
  }

  /**
   * 远程开锁
   */
  async unlock(password) {
    return this.sendAndWait(`CMD:UNLOCK,PWD=${password}\r\n`)
  }

  /**
   * 修改密码
   */
  async changePassword(oldPwd, newPwd) {
    return this.sendAndWait(`CMD:SETPWD,OLD=${oldPwd},NEW=${newPwd}\r\n`)
  }

  /**
   * 同步时间
   */
  async syncTime() {
    const now = new Date()
    const time = [
      String(now.getHours()).padStart(2, '0'),
      String(now.getMinutes()).padStart(2, '0'),
      String(now.getSeconds()).padStart(2, '0')
    ].join(':')
    const date = [
      String(now.getFullYear()).slice(2),
      String(now.getMonth() + 1).padStart(2, '0'),
      String(now.getDate()).padStart(2, '0')
    ].join('-')
    return this.send(`CMD:SYNC,TIME=${time},DATE=${date}\r\n`)
  }

  /**
   * 查询设备状态
   */
  async query() {
    return this.send('CMD:QUERY\r\n')
  }

  /**
   * 数据接收处理 (拼包 + 解析)
   */
  _onData(arrayBuffer) {
    const bytes = new Uint8Array(arrayBuffer)
    const str = String.fromCharCode.apply(null, bytes)
    this.dataBuffer += str

    while (this.dataBuffer.includes('\r\n')) {
      const idx = this.dataBuffer.indexOf('\r\n')
      const line = this.dataBuffer.substring(0, idx)
      this.dataBuffer = this.dataBuffer.substring(idx + 2)

      if (line.startsWith('SWB:')) {
        const data = this._parseSWB(line)
        if (data) {
          this._dataListeners.forEach(fn => fn(data))
        }
      } else if (line.startsWith('RSP:')) {
        this._handleResponse(line)
      }
    }
  }

  /**
   * 解析 SWB 遥测数据
   */
  _parseSWB(line) {
    try {
      const body = line.substring(4) // 去掉 "SWB:"
      const parts = body.split(',')
      const data = {}

      parts.forEach(part => {
        if (part.includes('=')) {
          const eqIdx = part.indexOf('=')
          const key = part.substring(0, eqIdx)
          const val = part.substring(eqIdx + 1)
          data[key] = isNaN(Number(val)) ? val : Number(val)
        } else if (/^\d{2}:\d{2}:\d{2}$/.test(part)) {
          data.time = part
        }
      })

      return {
        temperature: data.T != null ? data.T : null,
        tds: data.TDS != null ? data.TDS : null,
        waterLevel: data.WL != null ? data.WL : null,
        battery: data.BAT != null ? data.BAT : null,
        time: data.time || null,
        timestamp: Date.now()
      }
    } catch (e) {
      console.error('SWB 数据解析失败:', line, e)
      return null
    }
  }

  /**
   * 处理设备响应
   */
  _handleResponse(line) {
    // RSP:OK 或 RSP:ERR,CODE=01
    if (this._responseResolve) {
      const resolve = this._responseResolve
      this._responseResolve = null

      if (line === 'RSP:OK') {
        resolve({ success: true })
      } else {
        const codeMatch = line.match(/CODE=(\d+)/)
        resolve({
          success: false,
          code: codeMatch ? parseInt(codeMatch[1]) : -1,
          message: line
        })
      }
    }
  }

  /**
   * 注册数据监听
   */
  onData(callback) {
    this._dataListeners.push(callback)
  }

  offData(callback) {
    this._dataListeners = this._dataListeners.filter(fn => fn !== callback)
  }

  /**
   * 注册断连监听
   */
  onDisconnect(callback) {
    this._disconnectListeners.push(callback)
  }

  /**
   * 断开连接
   */
  async disconnect() {
    if (this.deviceId && this.connected) {
      try {
        await wx.closeBLEConnection({ deviceId: this.deviceId })
      } catch (e) {
        // 忽略断连错误
      }
      this.connected = false
      this.dataBuffer = ''
    }
  }

  /**
   * 关闭蓝牙适配器
   */
  async close() {
    await this.disconnect()
    try {
      await wx.closeBluetoothAdapter()
    } catch (e) {
      // 忽略
    }
  }
}

module.exports = new BLEManager()
