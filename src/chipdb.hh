/* Copyright (C) 2015 Cotton Seed
   
   This file is part of arachne-pnr.  Arachne-pnr is free software;
   you can redistribute it and/or modify it under the terms of the GNU
   General Public License version 2 as published by the Free Software
   Foundation.
   
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

#ifndef PNR_CHIPDB_HH
#define PNR_CHIPDB_HH

#include "location.hh"
#include "util.hh"
#include "hashmap.hh"

#include <ostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cassert>

class CBit
{
public:
  friend std::ostream &operator<<(std::ostream &s, const CBit &cbit);
  template<typename T> friend struct std::hash;
  
  int tile;
  int row;
  int col;
  
public:
  CBit()
    : tile(0), row(0), col(0)
  {}
  CBit(int tile_, int r, int c)
    : tile(tile_), row(r), col(c)
  {}
  
  bool operator==(const CBit &rhs) const;
  bool operator!=(const CBit &rhs) const { return !operator==(rhs); }
  
  bool operator<(const CBit &rhs) const;
};

namespace std {

template<>
struct hash<CBit>
{
public:
  size_t operator() (const CBit &cbit) const
  {
    std::hash<int> hasher;
    size_t h = hasher(cbit.tile);
    h = hash_combine(h, hasher(cbit.row));
    return hash_combine(h, hasher(cbit.col));
  }
};

}

class CBitVal
{
public:
  friend std::ostream &operator<<(std::ostream &s, const CBitVal &cbits);
  
  std::map<CBit, bool> cbit_val;
  
public:
  CBitVal() {}
  CBitVal(const std::map<CBit, bool> &cbv)
    : cbit_val(cbv)
  {}
  
  std::set<CBit> cbits() const;
};

class Switch
{
public:
  bool bidir; // routing
  int tile;
  int out;
  std::map<int, std::vector<bool>> in_val;
  std::vector<CBit> cbits;
  
public:
  Switch() {}
  Switch(bool bi,
	 int t,
	 int o,
	 const std::map<int, std::vector<bool>> &iv,
	 const std::vector<CBit> &cb)
    : bidir(bi),
      tile(t),
      out(o),
      in_val(iv),
      cbits(cb)
  {}
};

enum class TileType {
  // FIXME EMPTY, remove _TILE
  NO_TILE, IO_TILE, LOGIC_TILE, RAMB_TILE, RAMT_TILE,
};

enum class CellType : int {
  LOGIC, IO, GB, RAM, WARMBOOT, PLL,
};

constexpr int cell_type_idx(CellType type)
{
  return static_cast<int>(type);
}

static const int n_cell_types = cell_type_idx(CellType::PLL) + 1;

namespace std {

template<>
struct hash<TileType>
{
public:
  size_t operator() (TileType x) const
  {
    using underlying_t = typename std::underlying_type<TileType>::type;
    std::hash<underlying_t> hasher;
    return hasher(static_cast<underlying_t>(x));
  }
};

}

extern std::string tile_type_name(TileType t);

class Package
{
public:
  std::string name;
  
  std::map<std::string, Location> pin_loc;
  std::map<Location, std::string> loc_pin;
};

class ChipDB
{
public:
  std::string device;
  
  int width;
  int height;
  
  int n_tiles;
  int n_nets;
  int n_global_nets;
  
  std::map<std::string, Package> packages;
  
  std::map<Location, int> loc_pin_glb_num;
  
  std::vector<std::vector<int>> bank_tiles;
  
  std::vector<int> iolatch;  // tiles
  std::map<Location, Location> ieren;
  std::map<std::string, std::tuple<int, int, int>> extra_bits;
  
  std::map<std::pair<int, int>, int> gbufin;
  
  std::map<int, int> tile_colbuf_tile;
  
  std::vector<TileType> tile_type;
  std::vector<std::pair<int, std::string>> net_tile_name;
  std::vector<std::map<std::string, int>> tile_nets;
  
  // FIXME
  std::map<TileType,
	  std::map<std::string, std::vector<CBit>>>
    tile_nonrouting_cbits;
  
  std::vector<int> extra_cell_tile;
  std::vector<std::string> extra_cell_type;
  std::vector<std::map<std::string, std::pair<int, std::string>>>
    extra_cell_mfvs;
  
  std::vector<CellType> cell_type;
  std::vector<Location> cell_location;
  // FIXME loc_cell(const Location &loc), tile_pos_cell: tile -> pos -> cell
  HashMap<Location, int> loc_cell;
  std::vector<std::vector<int>> cell_type_cells;
  
  // buffers and routing
  std::vector<Switch> switches;
  
  std::vector<std::set<int>> out_switches;
  std::vector<std::set<int>> in_switches;
  
  std::map<TileType, std::pair<int, int>> tile_cbits_block_size;
  
  void add_cell(CellType type, const Location &loc);
  bool is_global_net(int i) const { return i < n_global_nets; }
  int find_switch(int in, int out) const;
  
  int tile(int x, int y) const
  {
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);
  
    return x + width*y;
  }
  
  int tile_x(int t) const
  {
    assert(t >= 0 && t <= n_tiles);
    return t % width;
  }

  int tile_y(int t) const
  {
    assert(t >= 0 && t <= n_tiles);
    return t / width;
  }
  
  int tile_bank(int t) const;
  
  void set_device(const std::string &d, int w, int h, int n_nets_);
  
public:
  ChipDB();
  
  void dump(std::ostream &s) const;
};

extern ChipDB *read_chipdb(const std::string &filename);

#endif
