/**
 * Copyright (c)  2022  Xiaomi Corporation (authors: Fangjun Kuang,
 *                                                   Wei Kang)
 *
 * See LICENSE for clarification regarding multiple authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sherpa/python/csrc/rnnt_conv_emformer_model.h"

#include <memory>
#include <string>
#include <tuple>

#include "sherpa/csrc/rnnt_conv_emformer_model.h"
#include "sherpa/csrc/rnnt_model.h"
#include "torch/torch.h"

namespace sherpa {

void PybindRnntConvEmformerModel(py::module &m) {  // NOLINT
  using PyClass = RnntConvEmformerModel;
  py::class_<PyClass, RnntModel>(m, "RnntConvEmformerModel")
      .def(py::init([](const std::string &filename,
                       py::object device = py::str("cpu"),
                       bool optimize_for_inference =
                           false) -> std::unique_ptr<PyClass> {
             std::string device_str =
                 device.is_none() ? "cpu" : py::str(device);
             return std::make_unique<PyClass>(
                 filename, torch::Device(device_str), optimize_for_inference);
           }),
           py::arg("filename"), py::arg("device") = py::str("cpu"),
           py::arg("optimize_for_inference") = false)
      .def(
          "encoder_streaming_forward",
          [](PyClass &self, const torch::Tensor &features,
             const torch::Tensor &features_length,
             const torch::Tensor &num_processed_frames,
             const PyClass::State &states)
              -> std::tuple<torch::Tensor, torch::Tensor, PyClass::State> {
            torch::Tensor encoder_out;
            torch::Tensor encoder_out_lens;
            torch::IValue next_states;

            std::tie(encoder_out, encoder_out_lens, next_states) =
                self.StreamingForwardEncoder(features, features_length,
                                             num_processed_frames,
                                             self.StateToIValue(states));

            return std::make_tuple(encoder_out, encoder_out_lens,
                                   self.StateFromIValue(next_states));
          },
          py::arg("features"), py::arg("features_length"),
          py::arg("num_processed_frames"), py::arg("states"),
          py::call_guard<py::gil_scoped_release>())
      .def(
          "get_encoder_init_states",
          [](PyClass &self) -> PyClass::State {
            auto ivalue = self.GetEncoderInitStates();
            return self.StateFromIValue(ivalue);
          },
          py::call_guard<py::gil_scoped_release>())
      .def_property_readonly("chunk_length", &PyClass::ChunkLength)
      .def_property_readonly("pad_length", &PyClass::PadLength);
}

}  // namespace sherpa
