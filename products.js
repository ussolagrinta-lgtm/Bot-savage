/** @type {import("./types").Product[]} */
const PRODUCTS = [
  {
    id: "usermode_source",
    name: "Source — User Mode",
    shortLabel: "Usermode source",
    description:
      "Full usermode source package. Budget tier with aggressive / destructive feature set.",
    priceUsd: 0,
    priceNote: "Cheap tier — ask in ticket",
    category: "Source",
  },
  {
    id: "kernel_source",
    name: "Source — Kernel Mode",
    shortLabel: "Kernel source",
    description: "Kernel-mode source package. Sold as-is; driver signing and deployment are on you.",
    priceUsd: 0,
    priceNote: "Quote in ticket",
    category: "Source",
  },
  {
    id: "streamproof_usermode",
    name: "Streamproof — Usermode (NVIDIA & AMD)",
    shortLabel: "Streamproof",
    description:
      "Usermode overlay / capture hiding build tested for NVIDIA and AMD GPUs (OBS, Discord, etc.).",
    priceUsd: 0,
    priceNote: "Quote in ticket",
    category: "Usermode",
  },
  {
    id: "fivem_072_dll",
    name: "0/72 Method DLL — FiveM",
    shortLabel: "0/72 FiveM DLL",
    description:
      "FiveM build using the 0/72 load method. Compatible game build is confirmed in ticket before delivery.",
    priceUsd: 0,
    priceNote: "FiveM — confirm build in ticket",
    category: "FiveM",
    tags: ["FiveM", "0/72"],
  },
];

function getProduct(productId) {
  return PRODUCTS.find((p) => p.id === productId) ?? null;
}

module.exports = { PRODUCTS, getProduct };