/**
 * 消息通知服务
 */
const notificationService = {
  // 订阅消息模板 ID (需在微信后台配置)
  TEMPLATE_DRINK_REMINDER: '', // 饮水提醒模板
  TEMPLATE_LOW_BATTERY: '',    // 低电量提醒模板
  TEMPLATE_QUALITY_ALERT: '',  // 水质预警模板

  /**
   * 请求订阅消息授权
   */
  async requestSubscribe(templateIds) {
    try {
      const res = await wx.requestSubscribeMessage({
        tmplIds: templateIds
      })
      return res
    } catch (e) {
      console.error('订阅消息授权失败:', e)
      return null
    }
  },

  /**
   * 请求饮水提醒授权
   */
  async requestDrinkReminder() {
    if (!this.TEMPLATE_DRINK_REMINDER) return null
    return this.requestSubscribe([this.TEMPLATE_DRINK_REMINDER])
  },

  /**
   * 检查并提示低电量
   */
  checkBattery(level) {
    if (level <= 10) {
      wx.showToast({
        title: '水杯电量不足10%，请及时充电',
        icon: 'none',
        duration: 3000
      })
    } else if (level <= 20) {
      wx.showToast({
        title: '水杯电量较低，请注意充电',
        icon: 'none',
        duration: 2000
      })
    }
  }
}

module.exports = notificationService
