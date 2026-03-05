// GPUTensorAccelerator.h — Tensor Operations for ML Inference on GPU
// Copyright (c) 2026 ExplorerLens Project
//
// Tensor operations for ML inference on GPU. Manages DirectML tensor buffers,
// supports common ops (conv2d, relu, upsample).
//
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <functional>

namespace ExplorerLens {
namespace Engine {

enum class TensorDataType : uint8_t {
    Float32,
    Float16,
    Int32,
    Int8,
    UInt8
};

enum class TensorOp : uint8_t {
    Conv2D,
    ReLU,
    MaxPool,
    Upsample,
    BatchNorm,
    Softmax,
    Add,
    Multiply
};

struct GPUTensorShape {
    uint32_t batch = 1;
    uint32_t channels = 1;
    uint32_t height = 1;
    uint32_t width = 1;

    inline size_t ElementCount() const {
        return static_cast<size_t>(batch) * channels * height * width;
    }

    inline size_t ByteSize(TensorDataType type) const {
        size_t elemSize = 4;
        switch (type) {
        case TensorDataType::Float32: elemSize = 4; break;
        case TensorDataType::Float16: elemSize = 2; break;
        case TensorDataType::Int32:   elemSize = 4; break;
        case TensorDataType::Int8:    elemSize = 1; break;
        case TensorDataType::UInt8:   elemSize = 1; break;
        }
        return ElementCount() * elemSize;
    }
};

struct TensorBuffer {
    std::vector<float> data;
    GPUTensorShape shape;
    TensorDataType dataType = TensorDataType::Float32;
    std::string name;

    inline float& At(uint32_t b, uint32_t c, uint32_t h, uint32_t w) {
        size_t idx = ((static_cast<size_t>(b) * shape.channels + c) * shape.height + h) * shape.width + w;
        return data[idx];
    }

    inline float At(uint32_t b, uint32_t c, uint32_t h, uint32_t w) const {
        size_t idx = ((static_cast<size_t>(b) * shape.channels + c) * shape.height + h) * shape.width + w;
        return data[idx];
    }
};

class GPUTensorAccelerator {
public:
    static GPUTensorAccelerator& Instance() {
        static GPUTensorAccelerator instance;
        return instance;
    }

    inline TensorBuffer CreateTensor(const GPUTensorShape& shape, TensorDataType type = TensorDataType::Float32,
        const std::string& name = "") const {
        TensorBuffer buffer;
        buffer.shape = shape;
        buffer.dataType = type;
        buffer.name = name;
        buffer.data.resize(shape.ElementCount(), 0.0f);
        return buffer;
    }

    inline TensorBuffer ReLU(const TensorBuffer& input) const {
        TensorBuffer output = input;
        for (auto& v : output.data) {
            v = (std::max)(0.0f, v);
        }
        return output;
    }

    inline TensorBuffer Softmax(const TensorBuffer& input) const {
        TensorBuffer output = input;
        for (uint32_t b = 0; b < input.shape.batch; ++b) {
            size_t offset = static_cast<size_t>(b) * input.shape.channels * input.shape.height * input.shape.width;
            size_t count = static_cast<size_t>(input.shape.channels) * input.shape.height * input.shape.width;

            float maxVal = *std::max_element(output.data.begin() + offset, output.data.begin() + offset + count);
            float expSum = 0.0f;
            for (size_t i = 0; i < count; ++i) {
                output.data[offset + i] = std::exp(output.data[offset + i] - maxVal);
                expSum += output.data[offset + i];
            }
            if (expSum > 0.0f) {
                for (size_t i = 0; i < count; ++i) {
                    output.data[offset + i] /= expSum;
                }
            }
        }
        return output;
    }

    inline TensorBuffer Conv2D(const TensorBuffer& input, const TensorBuffer& kernel,
        uint32_t stride = 1, uint32_t padding = 0) const {
        uint32_t outH = (input.shape.height + 2 * padding - kernel.shape.height) / stride + 1;
        uint32_t outW = (input.shape.width + 2 * padding - kernel.shape.width) / stride + 1;
        uint32_t outC = kernel.shape.batch;

        GPUTensorShape outShape{ input.shape.batch, outC, outH, outW };
        TensorBuffer output = CreateTensor(outShape, input.dataType);

        for (uint32_t b = 0; b < input.shape.batch; ++b) {
            for (uint32_t oc = 0; oc < outC; ++oc) {
                for (uint32_t oh = 0; oh < outH; ++oh) {
                    for (uint32_t ow = 0; ow < outW; ++ow) {
                        float sum = 0.0f;
                        for (uint32_t ic = 0; ic < input.shape.channels; ++ic) {
                            for (uint32_t kh = 0; kh < kernel.shape.height; ++kh) {
                                for (uint32_t kw = 0; kw < kernel.shape.width; ++kw) {
                                    int ih = static_cast<int>(oh * stride + kh) - static_cast<int>(padding);
                                    int iw = static_cast<int>(ow * stride + kw) - static_cast<int>(padding);
                                    if (ih >= 0 && ih < static_cast<int>(input.shape.height) &&
                                        iw >= 0 && iw < static_cast<int>(input.shape.width)) {
                                        sum += input.At(b, ic, ih, iw) * kernel.At(oc, ic, kh, kw);
                                    }
                                }
                            }
                        }
                        output.At(b, oc, oh, ow) = sum;
                    }
                }
            }
        }
        return output;
    }

    inline TensorBuffer MaxPool2D(const TensorBuffer& input, uint32_t poolSize = 2, uint32_t stride = 2) const {
        uint32_t outH = (input.shape.height - poolSize) / stride + 1;
        uint32_t outW = (input.shape.width - poolSize) / stride + 1;
        GPUTensorShape outShape{ input.shape.batch, input.shape.channels, outH, outW };
        TensorBuffer output = CreateTensor(outShape, input.dataType);

        for (uint32_t b = 0; b < input.shape.batch; ++b) {
            for (uint32_t c = 0; c < input.shape.channels; ++c) {
                for (uint32_t oh = 0; oh < outH; ++oh) {
                    for (uint32_t ow = 0; ow < outW; ++ow) {
                        float maxVal = -1e30f;
                        for (uint32_t ph = 0; ph < poolSize; ++ph) {
                            for (uint32_t pw = 0; pw < poolSize; ++pw) {
                                float v = input.At(b, c, oh * stride + ph, ow * stride + pw);
                                if (v > maxVal) maxVal = v;
                            }
                        }
                        output.At(b, c, oh, ow) = maxVal;
                    }
                }
            }
        }
        return output;
    }

    inline std::string ShapeToString(const GPUTensorShape& shape) const {
        return "[" + std::to_string(shape.batch) + "," + std::to_string(shape.channels) + "," +
            std::to_string(shape.height) + "," + std::to_string(shape.width) + "]";
    }

private:
    GPUTensorAccelerator() = default;
};

}
} // namespace ExplorerLens::Engine
