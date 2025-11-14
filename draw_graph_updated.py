import networkx as nx
import matplotlib.pyplot as plt
from networkx.drawing.nx_pydot import graphviz_layout

# --- 1. 定义节点和标签 ---
nodes = {
    'root': "135c4e764\n9611314",
    'dep_z': "027f4267d\n969e1b8",
    'api_ref': "9ea07b03\nb88cb526",
    'p1': "d04d4dd31\nefe67e85",
    'p2': "b08bc12b\nf8734c5a",
    'p3': "cf5908a52\ne0aee5e",
    'p4': "b7b85590b\n7d28ba6",
    'p5': "2b7cbdef32\n03a482",
    'p6': "4831f393a0\n066565"
}

G = nx.DiGraph()
G.add_nodes_from(nodes.keys())

# --- 2. 定义边 ---
z_edges = [
    ('root', 'dep_z'),
    ('root', 'p2'),
    ('root', 'p3'),
    ('root', 'p4'),
    ('root', 'p5'),
    ('root', 'p6')
]
G.add_edges_from(z_edges, color='skyblue', style='dashed', label='z')

wires_edges = [
    ('p1', 'p2'),
    ('p2', 'p3'),
    ('p3', 'p4'),
    ('p4', 'p5'),
    ('p5', 'p6'),
    ('dep_z', 'p4')
]
G.add_edges_from(wires_edges, color='red', style='solid', label='wires[0]', weight=2)

api_edge = ('p2', 'api_ref')
G.add_edge(*api_edge, color='green', style='solid', label='apiEndpointRef', weight=2)

# --- 3. 绘图设置 ---
node_color_map = ['#5a9bd6'] * len(G.nodes)
node_size = 4000

edge_colors = [G[u][v]['color'] for u, v in G.edges()]
edge_styles = [G[u][v]['style'] for u, v in G.edges()]
edge_weights = [G[u][v]['weight'] if 'weight' in G[u][v] else 1 for u, v in G.edges()]

# --- 4. 调整布局 ---
try:
    pos = graphviz_layout(G, prog='dot')
    pos = {k: (x * 2.0, y * 2.0) for k, (x, y) in pos.items()}  # 放大布局比例
except ImportError:
    pos = nx.spring_layout(G, seed=42, k=1.5)

# --- 5. 绘制 ---
plt.figure(figsize=(18, 10))

nx.draw_networkx_nodes(G, pos,
                       node_color=node_color_map,
                       node_shape='o',
                       node_size=node_size,
                       edgecolors='black')

nx.draw_networkx_edges(G, pos,
                       edge_color=edge_colors,
                       style=edge_styles,
                       width=edge_weights,
                       arrows=True,
                       arrowsize=25,
                       connectionstyle="arc3,rad=0.05")

nx.draw_networkx_labels(G, pos,
                        labels=nodes,
                        font_size=9,
                        font_color='white',
                        font_weight='bold')

edge_labels = nx.get_edge_attributes(G, 'label')
nx.draw_networkx_edge_labels(G, pos,
                             edge_labels=edge_labels,
                             font_size=10,
                             font_color='red',
                             bbox={'facecolor': 'white', 'alpha': 0.7, 'edgecolor': 'none'},
                             label_pos=0.5)

plt.axis('off')
plt.title("GUI)", fontsize=14)
plt.tight_layout()
plt.show()
