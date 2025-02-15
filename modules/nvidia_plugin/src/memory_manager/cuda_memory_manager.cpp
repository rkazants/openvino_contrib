// Copyright (C) 2018-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "cuda_memory_manager.hpp"

#include <details/ie_exception.hpp>

#include "cuda_operation_base.hpp"

namespace ov {
namespace nvidia_gpu {

MemoryManager::MemoryManager(DeviceMemBlock::Ptr immutableTensors,
                             MemoryModel::Ptr mutableMemoryModel,
                             DeviceMemBlock::Ptr immutableWorkbufferMemory)
    : immutable_tensors_{immutableTensors},
      mutable_tensors_model_{mutableMemoryModel},
      immutable_workbuffers_{immutableWorkbufferMemory} {}

MemoryManager::InputTensors MemoryManager::inputTensorPointers(const IOperationMeta& operation,
                                                               CUDA::DevicePointer<void*> mutableBufferPtr) const {
    InputTensors result;
    for (auto id : operation.GetInputIds()) {
        const void* ptr = immutable_tensors_->deviceTensorPtr(id);
        if (ptr == nullptr) ptr = mutable_tensors_model_->deviceTensorPtr(mutableBufferPtr.cast<uint8_t*>(), id);

        IE_ASSERT(ptr != nullptr) << "Tensor not found. ID is " << id;
        result.emplace_back(ptr);
    }
    return result;
}

MemoryManager::OutputTensors MemoryManager::outputTensorPointers(const IOperationMeta& operation,
                                                                 CUDA::DevicePointer<void*> mutableBufferPtr) const {
    OutputTensors result;
    for (auto id : operation.GetOutputIds()) {
        void* ptr = mutable_tensors_model_->deviceTensorPtr(mutableBufferPtr.cast<uint8_t*>(), id);

        IE_ASSERT(ptr != nullptr) << "Tensor not found. ID is " << id;
        result.emplace_back(ptr);
    }
    return result;
}

Workbuffers MemoryManager::workBuffers(const IOperationExec& operation,
                                       CUDA::DevicePointer<void*> mutableBufferPtr) const {
    Workbuffers result{};
    const auto& indices = operation.GetWorkbufferIds();
    for (const auto immutable_id : indices.immutableIds) {
        void* ptr = immutable_workbuffers_->deviceBufferPtr(immutable_id);
        IE_ASSERT(ptr != nullptr) << "Workbuffer not found. ID is " << immutable_id;
        result.immutable_buffers.emplace_back(ptr);
    }
    for (const auto mutable_id : indices.mutableIds) {
        void* ptr = mutable_tensors_model_->deviceBufferPtr(mutableBufferPtr.cast<uint8_t*>(), mutable_id);
        IE_ASSERT(ptr != nullptr) << "Workbuffer not found. ID is " << mutable_id;
        result.mutable_buffers.emplace_back(ptr);
    }
    return result;
}

}  // namespace nvidia_gpu
}  // namespace ov
