#pragma once
namespace asteria {
void draw_water_autotile(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw);
void draw_path_autotile (int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw);
void draw_natural_water(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw);
void draw_natural_path(int sx,int sy, bool n,bool s,bool e,bool w,bool ne,bool nw,bool se,bool sw);
void draw_roof_tile(int sx,int sy, bool up,bool down,bool left,bool right, int tileIdx);
void draw_wall_tile(int sx,int sy, bool up,bool down,bool left,bool right, int tileIdx);
void draw_rampart_tile(int sx,int sy, bool up,bool down,bool left,bool right);
void draw_tree(int sx,int sy);
void draw_tower(int sx,int sy);
bool is_facade_tile(int t);
int  facade_material(const unsigned char* M,int W,int H,int x,int y);
void draw_facade_tile(int sx,int sy, bool up,bool down,bool left,bool right, int selfTile,int matTile);
void draw_inn_tile(int sx,int sy,int t);   // mobilier taverne : 23 comptoir,25 tonneau,26 bouteilles,27 tabouret
void draw_table_tile(int sx,int sy,bool up,bool down,bool left,bool right);  // 24 grande table (multi-cellule)
}
