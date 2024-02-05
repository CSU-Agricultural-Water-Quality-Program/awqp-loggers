# Scalable Monitoring of Cercospora Leaf Spot and Temperature in Beet Storage Piles using Low-Cost, Internet of Things (IoT) Systems 

A multi-functional, low-cost, IoT data logger created for monitoring sugarbeet crop health and storage by the [Colorado State University (CSU) Agricultural Water Quality Program (AWQP)](https://waterquality.colostate.edu).

![data logger diagram](./figures/Cercospora-PT-joint-fig.jpg)

## Project Members

* A.J. Brown<sup>a</sup>, Agricultural Data Scientist, Ansley.Brown@colostate.edu
* Emmanuel Deleon<sup>a</sup>, Technical Research Lead, e.deleon@colostate.edu
* Erik Wardle<sup>a</sup>, Program Director, Erik.Wardle@colostate.edu

<sup>a</sup>Colorado State University Agricultural Water Quality Program

## Project Summary
As technology improves and scale, low-cost alternatives to traditional environmental sensors continue to emerge in the agricultural sector. In sugar beet production, two main operations have been identified by the CSU AWQP, in collaboration with Western Sugar (WS) as an opportunity to capitalize on integrating low-cost, Internet of Things (IoT) sensing into the sugar production process: 1) detecting sugar beet susceptibility to cercospora leaf spot (CLS), a common and detrimental leaf pest, and 2) detecting sugar loss in post-harvest sugar beet piles with temperature sensors. In 2022, the CSU AWQP prototyped and deployed four temperature (T) and relative humidity (RH) sensors to determine daily infection values (DIVs) for CLS with the help of the University of Nebraska-Lincoln collaborators. Results from previous years indicated that the sensors were performing adequately such that they could be a reasonable substitute for current methods of monitoring CLS and storage pile temperature (PT). As such, WS has expressed interest in finding scalable ways to increase the number of sensors in use, as well as determine long-term solutions for data storage. This proposal details how CSU AWQP intends to increase functionality of existing IoT sensors used by WS and create educational workshops to train WS in the production of said sensors for future use.

## Table of Contents
- [Repo Contents](folder-contents)
- [3d prints](#3d-prints)
- [Parts List](#parts-list)
- [How-to guide](#how-to-guide)
- [Known Bugs](#known-bugs)
- [Future Developments](#future-developments)

---

## Repo Contents
* awqp-loggers
* figures
    * figures embedded into README.md


## 3D Prints
* Radiation Sheild - [Thingiverse Link](https://www.thingiverse.com/thing:5825967)

## Parts List
**Single Items**
| Item Name | Price | Unit | $/Unit | Qty | Total/Sensor | Link |
|-----------|-------|------|--------|-----|--------------|------|
| SHT 31 Sensor Probe | $18.99 | each | $18.99 | 1 | $18.99 | [Amazon](https://www.amazon.com/gp/product/B09NYDCKF2/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1) |
| Waterproof Polarized 4-Wire Cable Set | $2.50 | each | $2.50 | 1 | $2.50 | [Adafruit](https://www.adafruit.com/product/744) |
| Adafruit Micro Lipo - USB LiIon/LiPoly Charger | $5.95 | each | $5.95 | 1 | $5.95 | [Adafruit](https://www.adafruit.com/product/1304) |
| Lithium Ion Battery Pack - 3.7V 4400mAh | $19.95 | each | $19.95 | 2 | $39.90 | [Adafruit](https://www.adafruit.com/product/354) |
| Grove Shield FeatherWing for Particle Mesh and all Feathers | $5.95 | each | $5.95 | 1 | $5.95 | [Adafruit](https://www.adafruit.com/product/4309) |
| Boron LTE CAT-M1 for North America (BRN404X) | $65.00 | each | $65.00 | 1 | $65.00 | [Particle](https://store.particle.io/collections/cellular/products/boron-lte-cat-m1-noram-with-ethersim-4th-gen) |
| Waterproof Enclosure (NEW) | $19.99 | each | $19.99 | 1 | $19.99 | [Amazon](https://www.amazon.com/dp/B0BZR3GZ4V/ref=twister_B0B87THLGC?_encoding=UTF8&psc=1) |

**Packs of Items (You have to buy at least one pack)**

| Item Name | Price | Unit | $/Unit | Qty | Total/Sensor | Link |
|-----------|-------|------|--------|-----|--------------|------|
| Silica Pack (30 pack) | $9.99 | 30 pk | $0.33 | 1 | $0.33 | [Amazon](https://www.amazon.com/Dry-Premium-Packets-Desiccants-Dehumidifier/dp/B00VJ02VMK/ref=sr_1_3?crid=27DLR5YA4ZQID&keywords=10g%2Bsilica%2Bgel%2Bpackets&qid=1675185727&sprefix=10%2Bg%2Bsilica%2B%2Caps%2C139&sr=8-3&th=1) |
| White Solder Seal Wire Connectors AWG26-24 | $33.99 | 500 pk | $0.07 | 4 | $0.27 | [Amazon](https://www.amazon.com/gp/product/B09BHW6BWC/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) |
| 1/4" Heat Shrink Tubing (50ft) | $21.55 | 50 ft | $0.43 | 1 | $0.43 | [Amazon](https://www.amazon.com/gp/product/B08WB2HR66/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1) |
| 22 Gauge 4 Conductor Wire (65 ft) | $27.88 | 65 ft | $0.43 | 7 | $3.00 | [Amazon](https://www.amazon.com/gp/product/B0BV2QHVMB/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) |
| Cable Gland (20 Pack) PG7 Waterproof | $7.99 | 20 pk | $0.40 | 1 | $0.40 | [Amazon](https://www.amazon.com/AMPELE-Waterproof-Plastic-Adjustable-3-5-6mm/dp/B08TCFM4C7/ref=sxts_b2b_sx_reorder_acb_customer?content-id=amzn1.sym.44ecadb3-1930-4ae5-8e7f-c0670e7d86ce%3Aamzn1.sym.44ecadb3-1930-4ae5-8e7f-c0670e7d86ce&crid=1OH62VJ42NK1U&cv_ct_cx=pg7%2Bcable%2Bgland&keywords=pg7%2Bcable%2Bgland&pd_rd_i=B09X4GK3FT&pd_rd_r=d5becbf4-0c5a-42a5-abc4-9cc85186de9b&pd_rd_w=O3qY1&pd_rd_wg=9baEq&pf_rd_p=44ecadb3-1930-4ae5-8e7f-c0670e7d86ce&pf_rd_r=2YZKZZNZ8KDT9W166GAZ&qid=1702330671&sbo=RZvfv%2F%2FHxDF%2BO5021pAnSA%3D%3D&sprefix=PG7%2B%2Caps%2C132&sr=1-1-62d64017-76a9-4f2a-8002-d7ec97456eea&th=1) |
| SeeedStudio Grove - Universal 4 Pin Buckled 20cm Cable (5 PCs Pack) | $8.59 | 5 pk | $1.72 | 1 | $1.72 | [Amazon](https://www.amazon.com/Seeedstudio-Grove-Universal-Buckled-Cable/dp/B01CNZ9RJO?th=1) |
| Fastening Cable Ties Reusable | $5.99 | 60 pk | $0.10 | 1 | $0.10 | [Amazon](https://www.amazon.com/gp/product/B07V73G556/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) |
| 3/4 in. 1-Hole Strap | $2.50 | 4 pk | $0.63 | 2 | $1.25 | [Home Depot](https://www.homedepot.com/p/Halex-Universal-3-4-in-1-Hole-Strap-4-Pack-22107/314968705?source=shoppingads&locale=en-US&pla&mtc=SHOPPING-BF-CDP-GGL-D27-027_006_CONDUIT_FIT-NA-NA-NA-PMAX-NA-NA-NA-NA-NBR-NA-NA-NEW-NA_D27Transition&cm_mmc=SHOPPING-BF-CDP-GGL-D27-027_006_CONDUIT_FIT-NA-NA-NA-PMAX-NA-NA-NA-NA-NBR-NA-NA-NEW-NA_D27Transition-71700000113157690--&gad_source=1&acs_info=ZmluYWxfdXJsOiAiaHR0cHM6Ly93d3cuaG9tZWRlcG90LmNvbS9wL0hhbGV4LVVuaXZlcnNhbC0zLTQtaW4tMS1Ib2xlLVN0cmFwLTQtUGFjay0yMjEwNy8zMTQ5Njg3MDUiCg&gclid=Cj0KCQiAh8OtBhCQARIsAIkWb6-ZNUGk4DF6_8K9b_SG3twg-_4rTYJGDSozB688e1TayXbsJCpro0IaAm47EALw_wcB&gclsrc=aw.ds) |
| Cone Drill bit or 1/2" drill bit | $9.99 | each | $9.99 | 1 | $9.99 | [Amazon](https://www.amazon.com/CO-Z-Titanium-Drilling-Cutting-Electrician/dp/B076QC5BWR/ref=sxin_14_pa_sp_search_thematic_sspa?content-id=amzn1.sym.92181fe7-c843-4c1b-b489-84c087a93895%3Aamzn1.sym.92181fe7-c843-4c1b-b489-84c087a93895&crid=2U01GZIGM67DL&cv_ct_cx=cone+drill+bit&keywords=cone+drill+bit&pd_rd_i=B076QC5BWR&pd_rd_r=557b2ca0-8959-460e-91cf-71f9f81a8c58&pd_rd_w=N3oPN&pd_rd_wg=O6cNx&pf_rd_p=92181fe7-c843-4c1b-b489-84c087a93895&pf_rd_r=WGYSVA5CQWX46TTP7JY3&qid=1706724546&sbo=RZvfv%2F%2FHxDF%2BO5021pAnSA%3D%3D&sprefix=cone+dril%2Caps%2C165&sr=1-5-364cf978-ce2a-480a-9bb0-bdb96faa0f61-spons&sp_csd=d2lkZ2V0TmFtZT1zcF9zZWFyY2hfdGhlbWF0aWM&psc=1) |

**Total:** $266.80 *(total including cost to buy bulk items)* | $175.78 *(total when accounting for using the exact number of parts)*


## How-To Guide

1. Purchase all necessary hardware components 
2. 3D-print necessary parts
3. Assemble hardware
4. Flash code
5. Using [particle console](https://console.particle.io/devices), create a webhook to your desired data collection platform (e.g., Ubidots, Azure, etc.) 
    * The CSU AWQP uses [Ubidots](https://industrial.ubidots.com/)
6. Check to ensure data is being transmitted correctly on data collection platform
7. Calculate DIVs using cloud-processing or download data and calculate locally, then deliver to stakeholders as needed for informed decision making.

## Known Bugs
* The solar panels only charge ~50% of the time.  We suspect the battery is too large, but this will require further testing. For now we are optimizing charging for a 2500 mAh LiPo, 3.7V battery.
    * As a result, we decided to not use the solar panels in the final design.

## Future Developments
* Cloud computing
    * Calculate cercospora daily infection values (DIVs) in the cloud on Ubidots and on Azure IoT Hub