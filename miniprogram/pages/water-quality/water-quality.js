const app = getApp()
const cloudService = require('../../services/cloud')
const { evaluateParam, evaluateOverall } = require('../../utils/water-quality')
const { formatTime } = require('../../utils/util')

Page({
  data: {
    tds: null,
    ec: null,
    salinity: null,
    hardness: null,
    tdsPercent: 0,
    ecPercent: 0,
    salinityPercent: 0,
    hardnessPercent: 0,
    tdsGrade: { label: '--', color: '#8C8C8C' },
    ecGrade: { label: '--', color: '#8C8C8C' },
    salinityGrade: { label: '--', color: '#8C8C8C' },
    hardnessGrade: { label: '--', color: '#8C8C8C' },
    overallGrade: { grade: '-', label: '无数据', color: '#8C8C8C' },
    updateTime: '--:--',
    period: '7',
    historyData: [],
    historyLabels: [],
    yMax: 600,
    yMid: 300,
    knowledgeList: [
      {
        title: 'TDS 是什么？',
        desc: 'TDS（Total Dissolved Solids）即总溶解固体，表示水中溶解物质的总量。一般饮用水TDS在0-300ppm为宜。'
      },
      {
        title: '什么是水的硬度？',
        desc: '水的硬度主要由钙、镁离子浓度决定。硬度过高可能导致水垢，建议饮用水硬度低于120ppm。'
      },
      {
        title: '电导率 EC 的含义',
        desc: '电导率反映水中离子含量，与TDS正相关。纯净水EC通常小于10us/cm，自来水在200-800us/cm。'
      },
      {
        title: '多久需要换水？',
        desc: '建议每次使用前换新鲜的水。如果水在杯中超过4小时，TDS值可能上升，建议重新换水。'
      }
    ]
  },

  onLoad() {
    this._dataHandler = (data) => this.onDeviceData(data)
    app.onDeviceData(this._dataHandler)
  },

  onShow() {
    this.updateFromDevice()
    this.loadHistory()
  },

  onUnload() {
    app.offDeviceData(this._dataHandler)
  },

  onDeviceData(data) {
    if (data.tds != null) {
      this.updateQualityDisplay({ tds: data.tds })
    }
  },

  updateFromDevice() {
    const deviceData = app.globalData.deviceData
    if (deviceData && deviceData.tds != null && deviceData.tds !== '--') {
      this.updateQualityDisplay({ tds: deviceData.tds })
    }
  },

  updateQualityDisplay(data) {
    const updates = { updateTime: formatTime() }

    if (data.tds != null) {
      updates.tds = data.tds
      updates.tdsGrade = evaluateParam(data.tds, 'tds')
      updates.tdsPercent = Math.min(100, Math.round(data.tds / 6))
    }
    if (data.ec != null) {
      updates.ec = data.ec
      updates.ecGrade = evaluateParam(data.ec, 'ec')
      updates.ecPercent = Math.min(100, Math.round(data.ec / 12))
    }
    if (data.salinity != null) {
      updates.salinity = (data.salinity * 0.01).toFixed(2)
      updates.salinityGrade = evaluateParam(data.salinity * 0.01, 'salinity')
      updates.salinityPercent = Math.min(100, Math.round(data.salinity * 0.01 / 0.002))
    }
    if (data.hardness != null) {
      updates.hardness = data.hardness
      updates.hardnessGrade = evaluateParam(data.hardness, 'hardness')
      updates.hardnessPercent = Math.min(100, Math.round(data.hardness / 1.8))
    }

    updates.overallGrade = evaluateOverall({
      tds: updates.tds || this.data.tds,
      ec: updates.ec || this.data.ec,
      salinity: updates.salinity || this.data.salinity,
      hardness: updates.hardness || this.data.hardness
    })

    this.setData(updates)
  },

  setPeriod(e) {
    this.setData({ period: e.currentTarget.dataset.period })
    this.loadHistory()
  },

  async loadHistory() {
    try {
      const days = parseInt(this.data.period)
      const data = await cloudService.getWaterQualityHistory(days)

      if (!data || data.length === 0) {
        this.setData({ historyData: [], historyLabels: [] })
        return
      }

      const maxTds = Math.max(...data.map(d => d.tds || 0), 300)
      const yMax = Math.ceil(maxTds / 100) * 100
      const yMid = Math.round(yMax / 2)

      const historyData = data.map(d => {
        const tds = d.tds || 0
        const grade = evaluateParam(tds, 'tds')
        return {
          date: d.recorded_at,
          tds,
          percent: Math.round(tds / yMax * 100),
          color: grade.color
        }
      })

      // 生成 X 轴标签 (均匀取几个日期)
      const labelCount = Math.min(7, data.length)
      const step = Math.max(1, Math.floor(data.length / labelCount))
      const historyLabels = []
      for (let i = 0; i < data.length; i += step) {
        const d = new Date(data[i].recorded_at)
        historyLabels.push(`${d.getMonth() + 1}/${d.getDate()}`)
      }

      this.setData({ historyData, historyLabels, yMax, yMid })
    } catch (e) {
      console.error('加载水质历史失败:', e)
    }
  }
})
