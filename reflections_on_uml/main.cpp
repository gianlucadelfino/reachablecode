#include <experimental/meta>
#include <iostream>
#include <utility>

class MyClass;
struct Nested
{
  int a;
  int b;
  const MyClass* test;
};

struct MyBase
{
  std::vector<int> base_member;
};

struct MyClass : MyBase {
  unsigned a = 1;
  unsigned i = 2;
  unsigned c =6;
  Nested const* n;
};

consteval std::string add_quotes(const auto& in)
{
  return std::string("\"") + in + std::string("\"");
}

consteval auto remove_ptr_cv_type_of(std::meta::info r) -> std::meta::info {
  return decay(remove_pointer(is_type(r) ? r : type_of(r)));
}

// @startuml
// foo o-left dummyLeft
// foo *-right dummyRight
// foo -up-> dummyUp
// foo -down-> dummyDown
// @enduml
consteval std::string make_class_graph_impl(std::meta::info head, std::vector<std::meta::info>& already_drawn)
{
  if(!std::meta::is_class_type(head))
  {
    return "";
  }

  if(std::find(already_drawn.cbegin(), already_drawn.cend(), head) != already_drawn.cend())
  {
    return "";
  }

  already_drawn.push_back(head);

  const std::string composition_arrow = "*--";
  const std::string inheritance_arrow = "-up-|>";


  constexpr auto ctx = std::meta::access_context::current();
  std::string uml_diagram;

  {
    uml_diagram += "together {\n class " + add_quotes(display_string_of(head)) + "\n";

    const std::string indent = "  ";

    // members
    for (std::meta::info field_info : std::meta::nonstatic_data_members_of(head, ctx))
    {
      uml_diagram += indent +  add_quotes(display_string_of(head)) + composition_arrow + add_quotes(display_string_of(remove_ptr_cv_type_of(field_info))) + "\n";
    }

    // Bases
    for (std::meta::info field_info : std::meta::bases_of(head, ctx))
    {
      uml_diagram += indent +  add_quotes(display_string_of(head)) + inheritance_arrow + add_quotes(display_string_of(remove_ptr_cv_type_of(field_info))) + "\n";
    }

    uml_diagram += "}\n";
  }

  // Recurse members
  for (std::meta::info field_info : std::meta::nonstatic_data_members_of(head, ctx))
  {
    if (std::meta::is_class_type(remove_ptr_cv_type_of(field_info)))
    {
      uml_diagram += make_class_graph_impl(remove_ptr_cv_type_of(field_info), already_drawn);
    }
  }

  // Recurse Bases
  for (std::meta::info field_info : std::meta::bases_of(head, ctx))
  {
    if (std::meta::is_class_type(remove_ptr_cv_type_of(field_info)))
    {
      uml_diagram += make_class_graph_impl(remove_ptr_cv_type_of(field_info), already_drawn);
    }
  }

  return uml_diagram;
}


template<typename U>
consteval const char* make_class_graph() {
  std::string graph = "@startuml \nskinparam linetype ortho \n";

  std::vector<std::meta::info> already_drawn;
  graph += make_class_graph_impl(^^U, already_drawn);
  graph += "@enduml";

  return std::define_static_string(graph);
}

int main() {
  MyClass s;
  constexpr const char* const dot_graph_uml = make_class_graph<MyClass>();

  std::cout << dot_graph_uml << std::endl;
}