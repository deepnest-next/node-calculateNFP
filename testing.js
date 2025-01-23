const addon = require("./build/Release/addon.node")

const A = {
	points: [
		{ x: 0, y: 0 },
		{ x: 20, y: 0 },
		{ x: 20, y: 20 },
		{ x: 0, y: 20 }
	],
	children: [
		[
			{ x: 5, y: 5 },
			{ x: 15, y: 5 },
			{ x: 15, y: 15 },
			{ x: 5, y: 15 }
		]
	]
}

const B = [
	{ x: 0, y: 0 },
	{ x: 5, y: 0 },
	{ x: 5, y: 5 },
	{ x: 0, y: 5 }
]

const hasHoles = true // Il poligono A ha un buco
const nfps = addon.calculateNFP({ A: A, B: B, hasHoles: hasHoles })

console.log(nfps)