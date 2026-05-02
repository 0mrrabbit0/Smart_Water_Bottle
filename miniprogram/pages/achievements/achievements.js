const cloudService = require('../../services/cloud')
const { ACHIEVEMENTS } = require('../../utils/constants')

Page({
  data: {
    achievementList: [],
    unlockedCount: 0,
    totalCount: ACHIEVEMENTS.length,
    progressPercent: 0
  },

  onShow() {
    this.loadAchievements()
  },

  async loadAchievements() {
    try {
      const unlocked = await cloudService.getAchievements()
      const unlockedMap = {}
      if (unlocked) {
        unlocked.forEach(a => {
          unlockedMap[a.achievement_id] = a.unlocked_at
        })
      }

      const achievementList = ACHIEVEMENTS.map(ach => {
        const isUnlocked = !!unlockedMap[ach.id]
        const unlockDate = unlockedMap[ach.id]
        return {
          ...ach,
          unlocked: isUnlocked,
          unlockTime: unlockDate ? this.formatUnlockTime(unlockDate) : ''
        }
      })

      // 已解锁的排前面
      achievementList.sort((a, b) => {
        if (a.unlocked && !b.unlocked) return -1
        if (!a.unlocked && b.unlocked) return 1
        return 0
      })

      const unlockedCount = achievementList.filter(a => a.unlocked).length

      this.setData({
        achievementList,
        unlockedCount,
        progressPercent: Math.round(unlockedCount / ACHIEVEMENTS.length * 100)
      })
    } catch (e) {
      console.error('加载成就失败:', e)
      // 显示全部成就为未解锁
      this.setData({
        achievementList: ACHIEVEMENTS.map(a => ({ ...a, unlocked: false }))
      })
    }
  },

  formatUnlockTime(dateStr) {
    const d = new Date(dateStr)
    return `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(2, '0')}-${String(d.getDate()).padStart(2, '0')} 达成`
  }
})
