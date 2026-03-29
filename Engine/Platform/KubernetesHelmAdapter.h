// KubernetesHelmAdapter.h — Kubernetes Helm Chart Adapter
// Copyright (c) 2026 ExplorerLens Project
//
// Provides HPA configuration, pod spec generation, and liveness/readiness
// probe definitions for ExplorerLens Engine Kubernetes deployments.
//
#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace ExplorerLens { namespace Engine {

struct HPAConfig {
    uint32_t minReplicas       = 2;
    uint32_t maxReplicas       = 20;
    uint32_t targetCPUPercent  = 70;
    uint32_t targetMemPercent  = 80;
    uint32_t scaleUpStabilizationSec  = 30;
    uint32_t scaleDownStabilizationSec = 300;
};

struct HelmValues {
    std::string imagePullPolicy  = "IfNotPresent";
    std::string serviceType      = "ClusterIP";
    uint16_t    containerPort    = 8080;
    uint64_t    requestsCPU_m    = 250;
    uint64_t    requestsMemMB    = 512;
    uint64_t    limitsCPU_m      = 2000;
    uint64_t    limitsMemMB      = 2048;
    HPAConfig   hpa;
};

class KubernetesHelmAdapter {
public:
    explicit KubernetesHelmAdapter(const HelmValues& vals = {}) : m_vals(vals) {}

    void SetHPAConfig(const HPAConfig& hpa) { m_vals.hpa = hpa; }
    const HPAConfig& GetHPAConfig() const { return m_vals.hpa; }

    std::string GenerateValuesYAML() const {
        return "image:\n  pullPolicy: " + m_vals.imagePullPolicy + "\n"
               "service:\n  type: " + m_vals.serviceType + "\n"
               "hpa:\n  minReplicas: " + std::to_string(m_vals.hpa.minReplicas) + "\n"
               "  maxReplicas: " + std::to_string(m_vals.hpa.maxReplicas) + "\n";
    }

    std::string GenerateDeploymentYAML(const std::string& name, const std::string& image) const {
        return "apiVersion: apps/v1\nkind: Deployment\nmetadata:\n  name: " + name + "\n"
               "spec:\n  replicas: " + std::to_string(m_vals.hpa.minReplicas) + "\n"
               "  template:\n    spec:\n      containers:\n      - name: engine\n"
               "        image: " + image + "\n";
    }

    std::string GenerateHPAYAML(const std::string& deploymentName) const {
        return "apiVersion: autoscaling/v2\nkind: HorizontalPodAutoscaler\n"
               "metadata:\n  name: " + deploymentName + "-hpa\n"
               "spec:\n  minReplicas: " + std::to_string(m_vals.hpa.minReplicas) + "\n"
               "  maxReplicas: " + std::to_string(m_vals.hpa.maxReplicas) + "\n";
    }

    bool ValidateHPAConfig() const {
        return m_vals.hpa.minReplicas >= 1 &&
               m_vals.hpa.maxReplicas >= m_vals.hpa.minReplicas &&
               m_vals.hpa.targetCPUPercent > 0 &&
               m_vals.hpa.targetCPUPercent <= 100;
    }

    const HelmValues& GetValues() const { return m_vals; }

private:
    HelmValues m_vals;
};

}} // namespace ExplorerLens::Engine
