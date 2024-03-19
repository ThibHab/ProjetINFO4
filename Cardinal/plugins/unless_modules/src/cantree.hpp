#include <iostream>

using namespace std;

namespace trees{

struct binode{
  float *value;
  float *raw_value;

  bool gate;
  int level;
  bool is_right = false;
  binode *left;
  binode *right;
  binode *parent;

  binode(float *v, float *rv, int l = 0, bool g = true, binode *p = NULL, bool r = false){
    value = v;
    raw_value = rv;
    gate = g;
    level = l;
    parent = p;
    left = NULL;
    right = NULL;
    is_right = r;
  }

  bool is_gate_upstream(bool g = true){
    if(g){
      if(parent)
        return parent->is_gate_upstream(gate);
      else
        return gate;
    }else
      return false;
  }

  int get_id(int l = 0, int i = 0){
    if(parent){
      if(is_right)
        return parent->get_id(l + 1, i + pow(2, l));
      else
        return parent->get_id(l + 1, i);
    }else{
      return i;
    }
  }

  json_t *toJson() {
    json_t *rootJ = json_object();
    json_object_set(rootJ, "gate", json_boolean(gate));
    json_object_set(rootJ, "is_right", json_boolean(is_right));
    if(left)
      json_object_set(rootJ, "left", left->toJson());
    if(right)
      json_object_set(rootJ, "right", right->toJson());

    return rootJ;
  }

  void fromJson(json_t *rootJ) {
    gate = json_boolean_value(json_object_get(rootJ, "gate"));
    is_right = json_boolean_value(json_object_get(rootJ, "is_right"));
    if(left)
      left->fromJson(json_object_get(rootJ, "left"));
    if(right)
      right->fromJson(json_object_get(rootJ, "right"));
  }

};

struct cantree{
  cantree(int depth, std::vector<float> *cs, std::vector<float> *cks);
  ~cantree();
  float left(float t, float c);
  json_t *toJson();
  void fromJson(json_t *rootJ);
  float time_of(int depth, binode *node, float t, int i);
  float right(float t, float c);
  void rec_fill(int depth, binode *node, int i);
  void rec_set_gate(int depth, binode *node, bool b, int i);
  void rec_set_single_gate(int level, int count, int index, bool b, binode *node, int i);
  bool is_gate(int depth, float time, bool absolute, bool with_mods, float pw);
  bool is_gate(int depth, float time, binode *node, bool with_mods, int i, float pw, bool absolute);
  void set_gate(bool b, int depth, float time, binode *node, bool with_mods, int i);
  void set_gate_at(int level, int index, bool b);
  binode *search(float key);
  void destroy_tree();
  void print();
  std::vector<float> *cuts;
  std::vector<float> *cut_knobs;
  binode *root;
  void print(binode *node);
  void destroy_tree(binode *leaf);
};

cantree::cantree(int depth, std::vector<float> *cs, std::vector<float> *cks){
  cuts = cs;
  cut_knobs = cks;
  root = new binode(&cuts->at(0), &cut_knobs->at(0));
  rec_fill(depth, root, 1);
}
float cantree::time_of(int depth, binode *node, float t, int i = 0){
  float c = *node->value;
  if(i == depth){
    if(t < c)
      return left(t, c);
    else
      return right(t, c);
  }else{
    if(t < c)
      return time_of(depth, node->left, left(t, c), i + 1);
    else
      return time_of(depth, node->right, right(t, c), i + 1);
  }
}
json_t *cantree::toJson() {
  return root->toJson();
}

void cantree::fromJson(json_t *rootJ) {
  if(root && rootJ)
    root->fromJson(rootJ);
}

void cantree::set_gate(bool b, int depth, float time, binode *node, bool with_mods = true, int i = 0){
  float c = with_mods ? *node->value : *node->raw_value;
  if(i == depth){
    if(time < c)
      node->left->gate = b;
    else
      node->right->gate = b;
  }else{
    if(time < c)
      set_gate(b, depth, left(time, c), node->left, with_mods, i + 1);
    else
      set_gate(b, depth, right(time, c), node->right, with_mods, i + 1);
  }
}

float cantree::left(float t, float c){
  return rescale(t, 0.0f, c, 0.0f, 1.0f);   
}

float cantree::right(float t, float c){
  return rescale(t, c, 1.0f, 0.0f, 1.0f);   
}


void cantree::rec_set_gate(int depth, binode *node, bool b, int i = 0){
  if(i == depth){
    node->left->gate = b;
    node->right->gate = b;
  }else{
    rec_set_gate(depth, node->left, b, i+1);
    rec_set_gate(depth, node->right, b, i+1);
  }
}

void cantree::rec_fill(int depth, binode *node, int i){
  if(i < depth){
    float *ck = &cut_knobs->at(i == depth - 1 ? i - 1 : i);
    node->left = new binode(&cuts->at(i), ck, i, true, node, false);
    node->right = new binode(&cuts->at(i), ck, i, true, node, true);
    rec_fill(depth, node->left, i + 1);
    rec_fill(depth, node->right, i + 1);
  }
}



void cantree::print(binode *node){
  if(node){
    printf("%d\n", node->level);
    print(node->left);
    print(node->right);
  }
}

void cantree::rec_set_single_gate(int level, int count, int index, bool b, binode *node, int i = 0){
  if(i == level - 1){
    if(index == 0)
      node->left->gate = b;
    else
      node->right->gate = b;
  }else{
    if(index < count / 2){
      rec_set_single_gate(level, count / 2, index, b, node->left, i + 1);
    }else{
      rec_set_single_gate(level, count / 2, index - (count / 2), b, node->right, i + 1);
    }
  }
}

void cantree::set_gate_at(int level, int index, bool b){
  // printf("level %d\n", );
  rec_set_single_gate(level + 1, pow(2, level + 1), index, b, root);
}

bool cantree::is_gate(int depth, float time, binode *node, bool with_mods = true, int i = -1, float pw = 0.5f, bool absolute = true){
  float c = with_mods ? *node->value : *node->raw_value;
  if(node->level == depth){
    if(time < c)
      return left(time, c) < pw && node->left->gate;
    else
      return right(time, c) < pw && node->right->gate;;
  }else{
    if(absolute){
      if(time < c)
        return is_gate(depth, left(time, c), node->left, with_mods, i + 1, pw);
      else
        return is_gate(depth, right(time, c), node->right, with_mods, i + 1, pw);
    }else{
      if(time < c){
        if(left(time, c) < pw && node->left->gate)
          return is_gate(depth, left(time, c), node->left, with_mods, i + 1, pw);
        else
          return false;
      }
      else
        if(right(time, c) < pw && node->right->gate)
          return is_gate(depth, right(time, c), node->right, with_mods, i + 1, pw);
        else
          return false;
    }
  }
}

bool cantree::is_gate(int depth, float time, bool absolute = true, bool with_mods = true, float pw = 0.5f){
  if(root){
    return is_gate(depth, time, root, with_mods, -1, pw, absolute);
  }
  return false;
}

void cantree::print(){
  print(root);
}

cantree::~cantree(){
  destroy_tree();
}

void cantree::destroy_tree(binode *leaf){
  if(leaf != NULL){
    destroy_tree(leaf->left);
    destroy_tree(leaf->right);
    delete leaf;
  }
}

void cantree::destroy_tree(){
  destroy_tree(root);
}
}
