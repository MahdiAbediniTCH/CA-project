import matplotlib.pyplot as plt

x_labels = ['1', '2', '4', '8', '16', '32']
x_positions = list(range(len(x_labels)))
trace = input('trace :')
cache_size = int(input('cache size :'))


lru_total = {
    'cassandra':[[899144, 911789, 903692, 909418, 907044, 905144],
            [908734, 906999, 910822, 914362, 913148, 914030],
                [908828, 917420, 911221, 915926, 916965, 917827]],
    'streaming':[[110065, 110354, 111708, 110603, 111564, 111187],
            [110408, 110432, 112161, 111859, 111777, 111248],
                [111748, 111240, 111594, 112019, 111193, 111108]]
}
lru_hits = {
    'cassandra':[[150948, 163735, 165909, 171383, 171894, 169697],
            [333933, 377982, 404945, 388625, 381830, 372830],
                [513752, 601363, 677387, 719518, 769128, 787084]],
    'streaming':[[20928, 23973, 27239, 29751, 31277, 31641],
            [34461, 40769, 46196, 47924, 49193 ,49466],
                [49286, 56684, 59962, 62478, 62195, 62684]]
}
lru_hit_rates = [100 * a / b for a, b in zip(lru_hits[trace][cache_size], lru_total[trace][cache_size])]

mru_total = {
    'cassandra':[[899144, 905102, 903156, 900671, 903445, 905463],
            [908734, 906881, 906466, 902278, 903278, 904663],
                [908828, 906408, 906527, 905069, 906831, 903887]],
    'streaming':[[110065, 109924, 108901, 109776, 109950, 110240],
            [110408, 110927, 109470, 110701, 108572, 109133],
                [111748, 110904, 110131, 109604, 110106, 109820]]
}
mru_hits = {
    'cassandra':[[150948, 69163, 17792, 11233, 10747, 14750],
            [333933, 175589, 76755, 35725, 23659, 23808],
                [513752, 356320, 198648, 97458, 57709, 45965]],
    'streaming':[[20928, 14620, 11057, 11106, 11630, 12333],
            [34461, 28158, 21990, 19414, 18054, 18483],
                [49286, 43479, 35365, 29740, 26776, 26401]]
}
mru_hit_rates = [100 * a / b for a, b in zip(mru_hits[trace][cache_size], mru_total[trace][cache_size])]

rnd_total = {
    'cassandra':[[899144, 907385, 910224, 913532, 910895, 912917],
            [908734, 913414, 908313, 917302, 908155, 914375],
                [908828, 911764, 913137, 907978, 910833, 910349]],
    'streaming':[[110065, 110691, 110045, 109968, 110027, 109924],
            [110408, 111121, 110892, 110302, 111516, 111104],
                [111748, 110514, 111110, 111413, 111114, 110566]]
}
rnd_hits = {
    'cassandra':[[150948, 137241, 150840, 187131, 215202, 230088],
            [333933, 272893, 219997, 249732, 275722, 305093],
                [513752, 442817, 388167, 338410, 335635, 349719]],
    'streaming':[[20928, 22262, 24207, 25347, 28202, 28760],
            [34461, 37798, 40746, 43599, 47653, 49710],
                [49286, 52204, 55355, 57572, 58113, 58958]]
}
rnd_hit_rates = [100 * a / b for a, b in zip(rnd_hits[trace][cache_size], rnd_total[trace][cache_size])]
# news
lfu_total = {
    'cassandra':[[899144, 909061, 910983, 911083, 914096, 916052],
            [908734, 909169, 915280, 914677, 911049, 914786],
                [908828,910525, 912061, 915302, 913983, 919720]],
    'streaming':[[110065, 110206, 110794,110434 ,110244, 110298],
            [110408, 109934, 111045, 110984, 111147, 111201],
                [111748, 111499, 110817, 111947, 112220, 111187]]
}
lfu_hits = {
    'cassandra':[[150948, 203751, 204492, 233765, 229664, 235183],
            [333933, 344520, 347428, 331705, 343497, 332935],
                [513752,553065, 519921, 487110, 463114, 461360]],
    'streaming':[[20928, 28132, 31687, 29584 , 28857, 29693],
            [34461, 46005, 50793, 52789, 52558, 51089],
                [49286, 61428, 64614, 67997, 67747, 64809]]
}
lfu_hit_rates = [100 * a / b for a, b in zip(lfu_hits[trace][cache_size], lfu_total[trace][cache_size])]

arc_total = {
    'cassandra':[[899144, 908005, 910387, 908568, 906889, 907822],
            [908734, 912275, 912457, 909465, 915206, 909375],
                [908828, 912119, 917139, 912933, 916304, 913884]],
    'streaming':[[110065, 110281, 110399, 110560, 111154, 111727],
            [110408, 110676, 111358, 111380, 111419, 111377],
                [111748, 111017, 111709, 112420, 112044, 112499]]
}
arc_hits = {
    'cassandra':[[150948, 163031, 169607, 167453, 169331, 167313],
            [333933, 367906, 396050, 393808, 386649, 371980],
                [513752, 598444, 674025, 730489, 742449, 779597]],
    'streaming':[[20928, 23854, 26623, 29149, 30772,31891],
            [34461, 41650, 44950, 47562, 48933, 49560],
                [49286, 56594, 60166, 62977, 63185, 64072]]
}
arc_hit_rates = [100 * a / b for a, b in zip(arc_hits[trace][cache_size], arc_total[trace][cache_size])]

srp_total = {
    'cassandra':[[899144, 908419, 913933, 914006, 909701, 913869],
            [908734, 918095, 909507, 920645, 909504, 908604],
                [908828, 911160, 910642, 918049, 913803, 918648]],
    'streaming':[[110065, 110812, 110797, 110559, 109329, 110497],
            [110408, 111609, 111098, 111428, 111719, 111185],
                [111748, 110721, 111621, 112067, 112190, 111006]]
}
srp_hits = {
    'cassandra':[[150948, 268986, 316942, 299602, 216377, 227542],
            [333933, 462749, 505550, 509860, 403013, 316307],
                [513752, 635480, 703086, 723737, 651188, 514264]],
    'streaming':[[20928, 30599, 34006, 31847, 28149, 29671],
            [34461, 46866, 51956, 54173, 53626, 51146],
                [49286, 60060, 65366, 68219, 67908, 65115]]
}
srp_hit_rates = [100 * a / b for a, b in zip(srp_hits[trace][cache_size], srp_total[trace][cache_size])]

tplru_total = {
    'cassandra':[[899144, 911789, 905902, 907896, 903422, 906098],
            [908734, 906999, 908675, 911509, 917828, 908429],
                [908828, 917420, 912628, 915228, 916226, 916152]],
    'streaming':[[110065, 110354, 110722, 111604, 110609, 111037],
            [110408, 110432, 111348, 112069, 110853, 112456],
                [111748, 111240, 111477, 112198, 111981, 111615]]
}
tplru_hits = {
    'cassandra':[[150948, 163735, 164683, 167942, 163551, 168558],
            [333933, 377982, 395818, 412203, 407160, 392977],
                [513752, 601363, 657653, 714504, 752319, 768190]],
    'streaming':[[20928, 23973, 26750, 29421, 29582, 31073],
            [34461, 40769, 45492, 47509, 47799, 49806],
                [49286, 56684, 59386, 61934, 62388, 62438]]
}
tplru_hit_rates = [100 * a / b for a, b in zip(tplru_hits[trace][cache_size], tplru_total[trace][cache_size])]

larc_total = {
    'cassandra':[[902820, 914804, 908919, 915724, 916408, 913748],
            [904624, 912981, 914278, 909632, 912998, 911702],
                [902515, 910911, 912691, 914384, 912221, 915184]],
    'streaming':[[110236, 110630, 110396, 111248, 111196, 112453],
            [110577, 110633, 112066, 111726, 111681, 112010],
                [109255, 112146, 111897, 111698, 111725, 110966]]
}
larc_hits = {
    'cassandra':[[14270, 338063, 366256, 389699, 412233, 414519],
            [28871, 497100, 534799, 529527, 545348, 543906],
                [59106, 658600, 721007, 756475, 780913, 798007]],
    'streaming':[[18904, 36824, 40423, 43542, 44511, 46802],
            [26800, 50228, 55338, 56356, 57166, 57908],
                [32115, 64357, 66718, 67600, 68754, 68526]]
}
larc_hit_rates = [100 * a / b for a, b in zip(larc_hits[trace][cache_size], larc_total[trace][cache_size])]


all_rates = (lru_hit_rates + mru_hit_rates + rnd_hit_rates + srp_hit_rates + arc_hit_rates + lfu_hit_rates
             + tplru_hit_rates + larc_hit_rates)
y_min = min(all_rates) - 2
y_max = max(all_rates) + 3

plt.figure(figsize=(8, 10))

plt.plot(x_positions, lfu_hit_rates, marker='o', linestyle='-', color='crimson', label='LFU')
plt.plot(x_positions, lru_hit_rates, marker='v', linestyle='-', color='teal', label='LRU')
plt.plot(x_positions, mru_hit_rates, marker='s', linestyle='--', color='orange', label='MRU')
plt.plot(x_positions, rnd_hit_rates, marker='^', linestyle='-.', color='green', label='Random')
plt.plot(x_positions, arc_hit_rates, marker='x', linestyle='-.', color='goldenrod', label='ARC')
plt.plot(x_positions, srp_hit_rates, marker='d', linestyle='-.', color='purple', label='SRRIP')
plt.plot(x_positions, tplru_hit_rates, marker='*', linestyle='-.', color='royalblue', label='TREE-PLRU')
plt.plot(x_positions, larc_hit_rates, marker='p', linestyle='-.', color='mediumseagreen', label='LARC')


for i in range(len(x_positions)):
    plt.text(x_positions[i], lru_hit_rates[i] + 0.5, f"{lru_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='teal')
    plt.text(x_positions[i], mru_hit_rates[i] + 0.5, f"{mru_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='orange')
    plt.text(x_positions[i], rnd_hit_rates[i] + 0.5, f"{rnd_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='green')
    plt.text(x_positions[i], lfu_hit_rates[i] + 0.5, f"{lfu_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='crimson')
    plt.text(x_positions[i], arc_hit_rates[i] + 0.5, f"{arc_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='goldenrod')
    plt.text(x_positions[i], srp_hit_rates[i] + 0.5, f"{srp_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='purple')
    plt.text(x_positions[i], tplru_hit_rates[i] + 0.5, f"{tplru_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='royalblue')
    plt.text(x_positions[i], larc_hit_rates[i] + 0.5, f"{larc_hit_rates[i]:.1f}%", ha='center', fontsize=8, color='mediumseagreen')

plt.xticks(ticks=x_positions, labels=x_labels)
plt.ylim(y_min, y_max)
plt.xlabel("Associativity")
plt.ylabel("Hit Rate (%)")
plt.title("Cache Replacement Policies ( " + str(2 ** (cache_size)) + " MB CACHE SIZE"+" - "+'Trace: '+trace+" )")
plt.legend()
plt.grid(True, which='both', linestyle='--', linewidth=0.5)
plt.minorticks_on()

plt.tight_layout()
plt.show()


