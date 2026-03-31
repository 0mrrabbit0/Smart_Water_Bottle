Component({
  properties: {
    current: {
      type: Number,
      value: 0
    },
    goal: {
      type: Number,
      value: 2000
    }
  },

  observers: {
    'current, goal': function () {
      this.drawRing()
    }
  },

  lifetimes: {
    ready() {
      this.drawRing()
    }
  },

  methods: {
    drawRing() {
      const query = this.createSelectorQuery()
      query.select('#progressCanvas')
        .fields({ node: true, size: true })
        .exec((res) => {
          if (!res[0]) return

          const canvas = res[0].node
          const ctx = canvas.getContext('2d')
          const dpr = wx.getWindowInfo().pixelRatio
          const width = res[0].width
          const height = res[0].height

          canvas.width = width * dpr
          canvas.height = height * dpr
          ctx.scale(dpr, dpr)

          const centerX = width / 2
          const centerY = height / 2
          const radius = Math.min(width, height) / 2 - 12
          const lineWidth = 12
          const progress = Math.min(this.data.current / this.data.goal, 1)

          // 清空
          ctx.clearRect(0, 0, width, height)

          // 背景环
          ctx.beginPath()
          ctx.arc(centerX, centerY, radius, 0, Math.PI * 2)
          ctx.strokeStyle = '#F0F0F0'
          ctx.lineWidth = lineWidth
          ctx.lineCap = 'round'
          ctx.stroke()

          // 进度环
          if (progress > 0) {
            const startAngle = -Math.PI / 2
            const endAngle = startAngle + Math.PI * 2 * progress

            // 渐变色
            const gradient = ctx.createLinearGradient(0, 0, width, height)
            if (progress >= 1) {
              gradient.addColorStop(0, '#52C41A')
              gradient.addColorStop(1, '#73D13D')
            } else {
              gradient.addColorStop(0, '#1890FF')
              gradient.addColorStop(1, '#69C0FF')
            }

            ctx.beginPath()
            ctx.arc(centerX, centerY, radius, startAngle, endAngle)
            ctx.strokeStyle = gradient
            ctx.lineWidth = lineWidth
            ctx.lineCap = 'round'
            ctx.stroke()
          }
        })
    }
  }
})
