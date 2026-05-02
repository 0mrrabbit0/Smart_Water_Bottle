const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()

exports.main = async (event, context) => {
  const { OPENID } = cloud.getWXContext()
  const date = event.date || new Date().toISOString().slice(0, 10)

  // 获取每日汇总
  const summaryRes = await db.collection('daily_summary')
    .where({ user_id: OPENID, date })
    .get()

  if (summaryRes.data.length === 0) {
    // 获取用户目标
    const userRes = await db.collection('users').where({ _id: OPENID }).get()
    const goal = (userRes.data[0] && userRes.data[0].daily_goal) || 2000

    return {
      total_amount: 0,
      drink_count: 0,
      goal: goal,
      goal_achieved: false,
      streak_days: 0,
      records: []
    }
  }

  const summary = summaryRes.data[0]

  // 获取当天记录
  const recordsRes = await db.collection('drink_records')
    .where({ user_id: OPENID, date_key: date })
    .orderBy('recorded_at', 'desc')
    .get()

  return {
    ...summary,
    records: recordsRes.data
  }
}
