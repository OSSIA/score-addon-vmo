#include "Process.hpp"

#include <wobjectimpl.h>

W_OBJECT_IMPL(vmo::Model)
namespace vmo
{

Model::Model(
    const TimeVal& duration,
    const Id<Process::ProcessModel>& id,
    QObject* parent)
    : Process::ProcessModel{duration, id, "vmoProcess", parent}
    , input{Id<Process::Port>(0), this}
    , regen{Id<Process::Port>(1), this}
    , bang{Id<Process::Port>(2), this}
    , sequence_length{Id<Process::Port>(3), this}
    , output{Id<Process::Port>(4), this}
{
  metadata().setInstanceName(*this);
  input.setCustomData("Input");
  regen.setCustomData("Regenerate");
  bang.setCustomData("Bang");
  sequence_length.setCustomData("Sequence Length");
  output.setCustomData("Input");
  m_inlets.push_back(&input);
  m_inlets.push_back(&regen);
  m_inlets.push_back(&bang);
  m_inlets.push_back(&sequence_length);
  m_outlets.push_back(&output);
}

Model::Model(DataStreamWriter& vis, QObject* parent)
    : Process::ProcessModel{vis, parent}
    , input{vis, parent}
    , regen{vis, parent}
    , bang{vis, parent}
    , sequence_length{vis, parent}
    , output{vis, parent}
{
  vis.writeTo(*this);
  m_inlets.push_back(&input);
  m_inlets.push_back(&regen);
  m_inlets.push_back(&bang);
  m_inlets.push_back(&sequence_length);
  m_outlets.push_back(&output);
}

Model::Model(JSONObject::Deserializer& vis, QObject* parent)
    : Process::ProcessModel{vis, parent}
    , input{JSONObject::Deserializer{vis.obj["Input"]}, parent}
    , regen{JSONObject::Deserializer{vis.obj["Regen"]}, parent}
    , bang{JSONObject::Deserializer{vis.obj["Bang"]}, parent}
    , sequence_length{JSONObject::Deserializer{vis.obj["SequenceLength"]},
                      parent}
    , output{JSONObject::Deserializer{vis.obj["Output"]}, parent}
{
  vis.writeTo(*this);
  m_inlets.push_back(&input);
  m_inlets.push_back(&regen);
  m_inlets.push_back(&bang);
  m_inlets.push_back(&sequence_length);
  m_outlets.push_back(&output);
}

Model::~Model() {}

QString Model::prettyName() const noexcept
{
  return tr("VMO Process");
}

}
template <>
void DataStreamReader::read(const vmo::Model& proc)
{
  m_stream << proc.input << proc.regen << proc.bang << proc.sequence_length
           << proc.output;
  insertDelimiter();
}

template <>
void DataStreamWriter::write(vmo::Model& proc)
{
  m_stream >> proc.input >> proc.regen >> proc.bang >> proc.sequence_length
      >> proc.output;
  checkDelimiter();
}

template <>
void JSONReader::read(const vmo::Model& proc)
{
  obj["Input"] = proc.input;
  obj["Regen"] = proc.regen;
  obj["Bang"] = proc.bang;
  obj["SequenceLength"] = proc.sequence_length;
  obj["Output"] = proc.output;
}

template <>
void JSONWriter::write(vmo::Model& proc)
{
}
