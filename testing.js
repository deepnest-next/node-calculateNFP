const addon = require("./build/Release/addon.node")

const A = {
	points: [
		{ X: 0, Y: 0 },
		{ X: 100, Y: 0 },
		{ X: 100, Y: 100 },
		{ X: 0, Y: 100 }
	],
	children: [
		[
			{ X: 10, Y: 10 },
			{ X: 30, Y: 10 },
			{ X: 30, Y: 30 },
			{ X: 10, Y: 30 },
		]
	]
}

const B = {
	points: [
		{ X: 100, Y: 0 },
		{ X: 150, Y: 0 },
		{ X: 125, Y: 25 },
	]
}

// A.children && A.children.length > 0
const hasHoles = A.children && A.children.length > 0
const nfps = addon.calculateNFP({ A: A, B: B, hasHoles: hasHoles });
console.dir(nfps, { depth: null, colors: true })