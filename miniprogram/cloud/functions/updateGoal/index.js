const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()

exports.main = async (event, context) => {
  const { OPENID } = cloud.getWXContext()
  const { goal } = event

  if (!goal || goal < 500 || goal > 5000) {
    return { success: false, message: '目标范围: 500-5000ml' }
  }

  // 更新用户表
  const userRes = await db.collection('users').where({ _id: OPENID }).get()
  if (userRes.data.length > 0) {
    await db.collection('users').doc(userRes.data[0]._id).update({
      data: { daily_goal: goal, updated_at: new Date() }
    })
  } else {
    await db.collection('users').add({
      data: {
        _id: OPENID,
        daily_goal: goal,
        created_at: new Date(),
        updated_at: new Date()
      }
    })
  }

  // 更新今日汇总中的目标
  const today = new Date().toISOString().slice(0, 10)
  const summaryRes = await db.collection('daily_summary')
    .where({ user_id: OPENID, date: today })
    .get()

  if (summaryRes.data.length > 0) {
    const summary = summaryRes.data[0]
    await db.collection('daily_summary').doc(summary._id).update({
      data: {
        goal: goal,
        goal_achieved: (summary.total_amount || 0) >= goal
      }
    })
  }

  return { success: true }
}
