const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()

exports.main = async (event, context) => {
  const { OPENID } = cloud.getWXContext()
  const type = event.type || 'daily'

  let dateFilter
  const today = new Date().toISOString().slice(0, 10)

  if (type === 'daily') {
    dateFilter = today
  } else {
    // 周排行：本周日期范围
    const now = new Date()
    const weekStart = new Date(now)
    weekStart.setDate(now.getDate() - now.getDay())
    dateFilter = weekStart.toISOString().slice(0, 10)
  }

  // 获取排行数据 (聚合查询)
  if (type === 'daily') {
    const res = await db.collection('daily_summary')
      .where({ date: today })
      .orderBy('total_amount', 'desc')
      .limit(50)
      .get()

    const ranking = res.data.map((d, i) => ({
      rank: i + 1,
      user_id: d.user_id,
      amount: d.total_amount,
      isMe: d.user_id === OPENID
    }))

    return { type, ranking }
  }

  // 连续达标天数排行
  const res = await db.collection('daily_summary')
    .where({ date: today })
    .orderBy('streak_days', 'desc')
    .limit(50)
    .get()

  const ranking = res.data.map((d, i) => ({
    rank: i + 1,
    user_id: d.user_id,
    streakDays: d.streak_days || 0,
    isMe: d.user_id === OPENID
  }))

  return { type: 'streak', ranking }
}
