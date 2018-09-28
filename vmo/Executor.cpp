#include "Executor.hpp"

#include <Process/ExecutionContext.hpp>

#include <ossia/dataflow/port.hpp>

#include <vmo/Process.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wregister"
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop
#include <ossia/network/value/value_hash.hpp>
#include <ossia/detail/hash_map.hpp>
namespace py = pybind11;

namespace vmo
{

py::scoped_interpreter& interpreter()
{
  static py::scoped_interpreter guard{};
  return guard;
}

py::module& oracle()
{
  interpreter();
  static py::module oracle{py::module::import("vmo.VMO.oracle")};
  return oracle;
}
py::module& generate()
{
  interpreter();
  static py::module oracle{py::module::import("vmo.generate")};
  return oracle;
}

class node final : public ossia::nonowning_graph_node
{
  ossia::inlet input{ossia::value_port{}};
  ossia::inlet regen{ossia::value_port{}};
  ossia::inlet bang{ossia::value_port{}};
  ossia::inlet sequence_length{ossia::value_port{}};
  ossia::outlet output{ossia::value_port{}};

  std::vector<ossia::value> input_sequence;
  std::vector<int> output_sequence;
  std::size_t sequence_idx{};
  int32_t m_sequence_length{16};

public:
  node()
  {
    input.data.target<ossia::value_port>()->is_event = true;
    regen.data.target<ossia::value_port>()->is_event = true;
    bang.data.target<ossia::value_port>()->is_event = true;
    m_inlets.push_back(&input);
    m_inlets.push_back(&regen);
    m_inlets.push_back(&bang);
    m_inlets.push_back(&sequence_length);
    m_outlets.push_back(&output);
  }

  void
  run(ossia::token_request tk, ossia::exec_state_facade s) noexcept override
  {
    interpreter();
    try
    {
      auto& i = *input.data.target<ossia::value_port>();
      auto& r = *regen.data.target<ossia::value_port>();
      auto& b = *bang.data.target<ossia::value_port>();
      auto& sl = *sequence_length.data.target<ossia::value_port>();
      auto& o = *output.data.target<ossia::value_port>();

      for(const auto& v : i.get_data())
      {
        input_sequence.push_back(v.value);
      }

      for(const auto& v : sl.get_data())
      {
        if(auto i = v.value.target<int32_t>())
          m_sequence_length = ossia::clamp(*i, 0, 10000);
      }

      if (!r.get_data().empty())
      {
        // Create a dictionary from the received values
        ossia::fast_hash_map<ossia::value, int> v;
        // VMO fails if alphabet starts at 0
        int max = 1;
        std::vector<int> seq;
        for(auto& val : input_sequence)
        {
          if(auto it = v.find(val); it != v.end())
          {
            seq.push_back(it->second);
          }
          else
          {
            v.insert({val, max});
            seq.push_back(max);
            max++;
          }
        }

        // Create a python oracle
        using namespace py::literals;
        auto p = oracle().attr("build_oracle")(seq, "a", 0.01);

        py::list res = generate().attr("improvise")(p, m_sequence_length);

        output_sequence.clear();
        for(auto item : res)
        {
          output_sequence.push_back(item.cast<int>()-1);
        }

        sequence_idx = 0;
      }

      if (!b.get_data().empty())
      {
        if(sequence_idx < output_sequence.size())
        {
          o.write_value(input_sequence[output_sequence[sequence_idx]], tk.offset);
        }
        sequence_idx = (sequence_idx + 1) % output_sequence.size();
      }
    }
    catch(std::exception& e)
    {
      std::cerr << e.what() << std::endl;
    }
  }

  std::string label() const noexcept override
  {
    return "VMO";
  }

private:
};

ProcessExecutorComponent::ProcessExecutorComponent(
    vmo::Model& element, const Execution::Context& ctx,
    const Id<score::Component>& id, QObject* parent)
    : ProcessComponent_T{element, ctx, id, "vmoExecutorComponent", parent}
{
  try
  {
    auto n = std::make_shared<vmo::node>();
    this->node = n;
    m_ossia_process = std::make_shared<ossia::node_process>(n);

  }
  catch(std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}
}
