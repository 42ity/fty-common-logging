#!/usr/bin/env groovy

@Library('etn-ipm2-jenkins@coverity_stub') _

import params.CmakePipelineParams
CmakePipelineParams parameters = new CmakePipelineParams()
//parameters.debugBuildRunMemcheck = false
//parameters.debugBuildRunCoverage = false

etn_ipm2_build_and_tests_pipeline_cmake(parameters)
