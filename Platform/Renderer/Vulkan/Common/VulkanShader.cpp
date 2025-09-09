//
// Created by IDKTHIS on 16.07.2025.
//

#include "VulkanShader.h"

#include <fstream>
#include <sstream>

#include "Core/Log/Logger.h"
#include "Platform/Renderer/Vulkan/Devices/LogicalDevice.h"


VulkanShader::VulkanShader(LogicalDevice* logicalDevice)
    : m_logicalDevice(logicalDevice) {}

VulkanShader::~VulkanShader() {
    for (auto& [_, module] : m_shaderModules) {
        m_logicalDevice->GetHandle().destroyShaderModule(module);
    }
}

std::unordered_map<std::string, ShaderStage> VulkanShader::StringToShaderStage() {
    return {
            {"vertex", ShaderStage::Vertex},
            {"fragment", ShaderStage::Fragment},
            {"compute", ShaderStage::Compute}
    };
}

shaderc_shader_kind VulkanShader::GetShadercKind(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:   return shaderc_glsl_vertex_shader;
        case ShaderStage::Fragment: return shaderc_glsl_fragment_shader;
        case ShaderStage::Compute:  return shaderc_glsl_compute_shader;
        default: throw std::runtime_error("Unsupported shader stage");
    }
}

vk::ShaderStageFlagBits VulkanShader::ConvertToShaderStage(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:   return vk::ShaderStageFlagBits::eVertex;
        case ShaderStage::Fragment: return vk::ShaderStageFlagBits::eFragment;
        case ShaderStage::Compute:  return vk::ShaderStageFlagBits::eCompute;
        default: throw std::runtime_error("Unsupported shader stage");
    }
}

std::vector<uint32_t> VulkanShader::CompileGLSL(const std::string& source, shaderc_shader_kind kind) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetTargetSpirv(shaderc_spirv_version_1_6);
    options.SetSourceLanguage(shaderc_source_language_glsl);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetWarningsAsErrors();
    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
        source,
        kind,
        "shader",
        options
    );

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG_ERROR("Shader", "Shader compilation failed: {}\n", result.GetErrorMessage().c_str());
        throw std::runtime_error("Shader compilation failed: " + result.GetErrorMessage());
    }
    return {result.cbegin(), result.cend()};
}

void VulkanShader::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Shader", "Failed to open shader file: {}", filepath);
        throw std::runtime_error("Failed to open shader file: " + filepath);
    }

    char bom[3] = {0};
    file.read(bom, 3);
    if (!(bom[0] == char(0xEF) && bom[1] == char(0xBB) && bom[2] == char(0xBF))) {
        file.seekg(0);
    }

    std::string fileContent((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    LoadFromSource(fileContent);
}

void VulkanShader::LoadFromSource(const std::string& source)
{
    std::unordered_map<std::string, std::string> sources;
    std::stringstream currentSource;
    std::string currentType;
    std::istringstream iss(source);
    std::string line;

    while (std::getline(iss, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        if (line.rfind("#type", 0) == 0) {
            if (!currentType.empty()) {
                sources[currentType] = currentSource.str();
                currentSource.str("");
                currentSource.clear();
            }

            currentType = line.substr(5);
            currentType.erase(0, currentType.find_first_not_of(" \t\n\r"));
            currentType.erase(currentType.find_last_not_of(" \t\n\r") + 1);
            std::transform(currentType.begin(), currentType.end(), currentType.begin(), ::tolower);
        } else {
            currentSource << line << '\n';
        }
    }
    if (!currentType.empty()) {
        sources[currentType] = currentSource.str();
    }

    auto stageMap = StringToShaderStage();

    for (const auto& [typeStr, src] : sources) {
        if (!stageMap.contains(typeStr)) {
            LOG_ERROR("Shader", "Unknown shader type encountered: '{}'", typeStr);
            continue;
        }

        ShaderStage stage = stageMap.at(typeStr);
        auto spirv = CompileGLSL(src, GetShadercKind(stage));

        vk::ShaderModuleCreateInfo info{};
        info.codeSize = spirv.size() * sizeof(uint32_t);
        info.pCode = spirv.data();

        vk::ShaderModule module = m_logicalDevice->GetHandle().createShaderModule(info);

        m_spirvCode[stage] = spirv;
        m_shaderModules[stage] = module;
    }
}

const std::unordered_map<ShaderStage, std::vector<uint32_t>>& VulkanShader::GetBinaryData() const {
    return m_spirvCode;
}

const std::unordered_map<ShaderStage, vk::ShaderModule>& VulkanShader::GetShaderModules() const {
    return m_shaderModules;
}

vk::ShaderModule& VulkanShader::GetShaderModule(const std::string &stageName) {
    auto map = StringToShaderStage();
    auto it = map.find(stageName);
    if (it == map.end()) {
        LOG_ERROR("Shader", "Unknown shader stage: '{}'", stageName);
throw std::runtime_error("Unknown shader stage: " + stageName);
    }

    ShaderStage stage = it->second;
    auto moduleIt = m_shaderModules.find(stage);
    if (moduleIt == m_shaderModules.end()) {
        throw std::runtime_error("Shader module for stage '" + stageName + "' not found");
    }

    return moduleIt->second;
}
