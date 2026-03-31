const { formatDate } = require('../utils/util')

/**
 * 云开发 API 封装
 */
const cloudService = {
  /**
   * 获取或创建用户信息
   */
  async getUser() {
    const db = wx.cloud.database()
    const { data } = await db.collection('users').where({
      _id: '{openid}'
    }).get()
    return data[0] || null
  },

  /**
   * 更新用户信息
   */
  async updateUser(userInfo) {
    const db = wx.cloud.database()
    const user = await this.getUser()
    if (user) {
      return db.collection('users').doc(user._id).update({
        data: {
          ...userInfo,
          updated_at: new Date()
        }
      })
    } else {
      return db.collection('users').add({
        data: {
          ...userInfo,
          created_at: new Date(),
          updated_at: new Date()
        }
      })
    }
  },

  /**
   * 添加饮水记录
   */
  async addDrinkRecord(record) {
    return wx.cloud.callFunction({
      name: 'addDrinkRecord',
      data: {
        amount: record.amount,
        source: record.source || 'manual',
        temperature: record.temperature,
        tds: record.tds,
        water_level_before: record.waterLevelBefore,
        water_level_after: record.waterLevelAfter
      }
    })
  },

  /**
   * 获取今日饮水记录
   */
  async getTodayRecords() {
    const db = wx.cloud.database()
    const today = formatDate()
    const { data } = await db.collection('drink_records')
      .where({ date_key: today })
      .orderBy('recorded_at', 'desc')
      .get()
    return data
  },

  /**
   * 获取每日汇总
   */
  async getDailySummary(date) {
    const result = await wx.cloud.callFunction({
      name: 'getDailySummary',
      data: { date: date || formatDate() }
    })
    return result.result
  },

  /**
   * 获取周统计
   */
  async getWeeklyStats() {
    const result = await wx.cloud.callFunction({
      name: 'getWeeklyStats'
    })
    return result.result
  },

  /**
   * 获取月统计
   */
  async getMonthlyStats(month) {
    const result = await wx.cloud.callFunction({
      name: 'getMonthlyStats',
      data: { month }
    })
    return result.result
  },

  /**
   * 更新每日目标
   */
  async updateGoal(goal) {
    return wx.cloud.callFunction({
      name: 'updateGoal',
      data: { goal }
    })
  },

  /**
   * 获取排行榜
   */
  async getRanking(type) {
    const result = await wx.cloud.callFunction({
      name: 'getRanking',
      data: { type: type || 'daily' }
    })
    return result.result
  },

  /**
   * 获取成就列表
   */
  async getAchievements() {
    const db = wx.cloud.database()
    const { data } = await db.collection('achievements')
      .orderBy('unlocked_at', 'desc')
      .get()
    return data
  },

  /**
   * 保存水质记录
   */
  async addWaterQualityLog(qualityData) {
    const db = wx.cloud.database()
    return db.collection('water_quality_log').add({
      data: {
        ...qualityData,
        recorded_at: new Date()
      }
    })
  },

  /**
   * 获取水质历史
   */
  async getWaterQualityHistory(days) {
    const db = wx.cloud.database()
    const startDate = new Date()
    startDate.setDate(startDate.getDate() - (days || 7))

    const { data } = await db.collection('water_quality_log')
      .where({
        recorded_at: db.command.gte(startDate)
      })
      .orderBy('recorded_at', 'asc')
      .limit(100)
      .get()
    return data
  }
}

module.exports = cloudService
