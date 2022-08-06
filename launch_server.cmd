@ECHO OFF
REM
REM Copyright (c) Contributors to the Open 3D Engine Project.
REM For complete copyright and license terms please see the LICENSE at the root of this distribution.
REM
REM SPDX-License-Identifier: Apache-2.0 OR MIT
REM
REM
REM Continuous Integration CLI entrypoint script to start CTest, triggering post-build tests
REM

.\build\windows\bin\profile\MultiplayerSample.ServerLauncher.exe --console-command-file=launch_server.cfg