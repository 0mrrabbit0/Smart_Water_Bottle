const cloud = require('wx-server-sdk')
cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV })
const db = cloud.database()
const _ = db.command

exports.main = async (event, context) => {
  const { OPENID } = cloud.getWXContext()
  const newAchievements = []

  // 获取已解锁成就
  const achRes = await db.collection('achievements')
    .where({ user_id: OPENID })
    .get()
  const unlocked = new Set(achRes.data.map(a => a.achievement_id))

  // 获取今日汇总
  const today = new Date().toISOString().slice(0, 10)
  const summaryRes = await db.collection('daily_summary')
    .where({ user_id: OPENID, date: today })
    .get()
  const todaySummary = summaryRes.data[0]

  // 1. 初次饮水
  if (!unlocked.has('first_drink')) {
    const recordCount = await db.collection('drink_records')
      .where({ user_id: OPENID })
      .count()
    if (recordCount.total > 0) {
      newAchievements.push('first_drink')
    }
  }

  // 2. 连续7天达标
  if (!unlocked.has('streak_7') && todaySummary && todaySummary.streak_days >= 7) {
    newAchievements.push('streak_7')
  }

  // 3. 连续30天达标
  if (!unlocked.has('streak_30') && todaySummary && todaySummary.streak_days >= 30) {
    newAchievements.push('streak_30')
  }

  // 4. 水质达人 (100次检测)
  if (!unlocked.has('quality_100')) {
    const wqCount = await db.collection('water_quality_log')
      .where({ user_id: OPENID })
      .count()
    if (wqCount.total >= 100) {
      newAchievements.push('quality_100')
    }
  }

  // 5. 千杯纪念
  if (!unlocked.has('drink_1000')) {
    const drinkCount = await db.collection('drink_records')
      .where({ user_id: OPENID })
      .count()
    if (drinkCount.total >= 1000) {
      newAchievements.push('drink_1000')
    }
  }

  // 写入新成就
  const now = new Date()
  for (const achId of newAchievements) {
    await db.collection('achievements').add({
      data: {
        user_id: OPENID,
        achievement_id: achId,
        unlocked_at: now
      }
    })
  }

  return { new_achievements: newAchievements }
}
