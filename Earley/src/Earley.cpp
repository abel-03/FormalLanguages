#include "./Earley.hpp"

std::vector<Rule> Grammar::starting_with(const std::string& start) const {
  std::vector<Rule> res;
  for (const auto& rule : rules_) {
    if (start == rule.left()) {
      res.push_back(rule);
    }
  } 
  return res;
}

Grammar& Parser::grammar() { return grammar_; }
const Grammar& Parser::grammar() const { return grammar_; }
const std::string& Parser::word() const { return word_; }
std::string& Parser::word() { return word_; }
Grammar::Grammar(const std::vector<Rule>& rules,
                 const std::set<std::string>& alph,
                 const std::set<std::string>& net, std::string start) {
  rules_ = rules;
  alphabet_ = alph;
  size_ = rules_.size();
  neterminals_ = net;
  start_ = start;
}

std::istream& operator>>(std::istream& in, Grammar& grammar) {
  std::cout << "Enter alphabet size\n";
  size_t size = 0;
  in >> size;
  grammar.size_ = size;
  char symbol = 0;
  std::cout << "Enter alphabet\n";
  for (size_t i = 0; i < size; ++i) {
    in >> symbol;
    grammar.alphabet_.insert(std::string(1, symbol));
  }
  std::cout << "Neterminals set size?\n";
  in >> size;
  std::cout << "Enter neterminals\n";
  for (size_t i = 0; i < size; ++i) {
    in >> symbol;
    grammar.neterminals_.insert(std::string(1, symbol));
  }
  std::cout << "Which neterminal is start?\n";
  in >> symbol;
  grammar.start_ = std::string(1, symbol);
  std::cout << "How many rules are there?\n";
  in >> size;
  std::cout << "Enter rules !! use this symbol for delimiter!!! →\n";
  grammar.rules_ = std::vector<Rule>(size);
  for (auto& elem : grammar.rules_) {
    in >> elem;
  }
  return in;
}

void Grammar::insert(Rule& new_rule) {
  ++size_;
  auto ps = neterminals_.find(new_rule.left());
  if (ps == neterminals_.end()) {
    neterminals_.insert(new_rule.left());
  }
  for (auto& elem : new_rule.right()) {
    auto aps = alphabet_.find(std::string(1, elem));
    if (aps == alphabet_.end() && islower(elem)) {
      abort();
    }
    auto nps = neterminals_.find(std::string(1, elem));
    if (nps == neterminals_.end()) {
      neterminals_.insert(std::string(1, elem));
    }
  }
}

Rule::Rule(const char* str) {
  std::string string(str);
  *this = string;
}

Rule& Rule::operator=(const Rule& other) {
  left_part_ = other.left_part_;
  right_part_ = other.right_part_;
  return *this;
}

Rule& Rule::operator=(const std::string& src) {
  *this = Rule(src);
  return *this;
}

Rule& Rule::operator=(const char* src) {
  std::string name = src;
  return *this = name;
}

Rule::Rule(const std::string& src) {
  std::string start = src.substr(0, src.find(delimiter));
  left_part_ = start;
  right_part_ = src.substr(src.find(delimiter) + delimiter.size());
}

bool Rule::operator<(const Rule& other) const {
  if (left_part_ == other.left_part_) {
    return right_part_ < other.right_part_;
  }
  return left_part_ < other.left_part_;
}

bool Rule::operator==(const Rule& other) const {
  return (!(*this < other) && !(other < *this));
}

std::istream& operator>>(std::istream& in, Rule& rule) {
  std::string r;
  in >> r;
  rule = Rule(r);
  return in;
}

State::State(const Rule& rule_, const size_t& pos_, const size_t& start_)
    : rule_(rule_), point_(pos_), start_(start_) {
  left_ = rule_.left();
  right_ = rule_.right();
}

bool State::operator==(const State& other) const {
  bool res = (rule_ == other.rule()) && (point_ == other.point()) &&
             (start_ == other.start());
  return res;
}

bool Parser::belongs() {
  build();
  std::string checkstr = def_start + "→" + grammar_.start();
  Rule check = checkstr;
  State sit(check, 1, 0);
  auto it = std::find(situations_[word_.size()].begin(),
                      situations_[word_.size()].end(), sit);
  return it != situations_[word_.size()].end();
}

bool State::operator<(const State& other) const {
  if (this->rule() == other.rule()) {
    if (point_ == other.point()) {
      return start() < other.start();
    }
    return point() < other.point();
  }
  return this->rule() < other.rule();
}

void Parser::build() {
  situations_.resize(word_.size() + 1);
  situations_[0].emplace(def_first_, 0, 0);
  int64_t size = -1;
  for (size_t i = 0; i < word_.size() + 1; ++i) {
    scan(i);
    size = -1;
    while (size != situations_[i].size()) {
      size = situations_[i].size();
      complete(i);
      predict(i);
    }
  }
}

Parser::Parser(const Grammar& grammar, const std::string& word)
    : grammar_(grammar), word_(word) {
  std::string checkstr = def_start + "→" + grammar_.start();
  def_first_ = checkstr;
  grammar_.insert(def_first_);
  grammar_.neterminals().insert(def_start);
}

void Parser::add(size_t j, const Rule& rule, size_t point, size_t index) {
  situations_[j].emplace(rule, point, index);
}
void Parser::scan(size_t j) {
  if (j == 0) {
    return;
  }
  for (const auto& elem : situations_[j - 1]) {
    if (std::string(1, word_[j - 1]) == elem.current()) {
      add(j, elem.rule(), elem.point() + 1, elem.start());
    }
  }
}
void Parser::complete(size_t j) {
  for (const auto& situation : situations_[j]) {
    if (situation.succeeded()) {
      for (auto& prev : situations_[situation.start()]) {
        if (prev.current() == situation.rule().left()) {
          add(j, prev.rule(), prev.point() + 1, prev.start());
        }
      } 
    }
  }
}
void Parser::predict(size_t j) {
  for (const auto& situation : situations_[j]) {
    std::string start = situation.current();
    for (const auto& elem : grammar_.starting_with(start)) {
      add(j, elem, 0, j);
    }
  }
}

int main() {
  Grammar grammar;
  std::cin >> grammar;
  std::cout << "Enter the word\n";
  std::string end;
  std::cin >> end;
  Parser parser(grammar, end);
  std::string word = parser.word();
  Grammar gram;
  const Grammar const_grammar = parser.grammar();
  gram = parser.grammar();
  std::cout << parser.belongs();
  parser.word() = word;
  parser.grammar() = gram;
  std::vector<Rule> rules = {Rule("S→AB"), Rule("A→aA"), Rule("B→b"),
                             Rule("A→a")};
  std::set<std::string> alph = {"a", "b"};
  Rule rule;
  rule = rules[0];
  std::set<std::string> net = {"S", "A", "B"};
  std::string start = "S";
  Grammar grammar2(rules, alph, net, start);
  State state;
  State state2 = state;
  return 0;
}
