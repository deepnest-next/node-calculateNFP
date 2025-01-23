const addon = require("./build/Release/addon.node")

// const result = addon.calculateNFP({
//     A: [{ x: 0, y: 0 }, { x: 10, y: 0 }, { x: 10, y: 10 }, { x: 0, y: 10 }],
//     B: [{ x: 0, y: 0 }, { x: 2, y: 0 }, { x: 2, y: 2 }, { x: 0, y: 2 }]
// })

const result = addon.calculateNFP(2, 3)

console.log(result)