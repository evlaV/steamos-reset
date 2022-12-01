// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

#ifndef CUSTOM_SCHEME_H_
#define CUSTOM_SCHEME_H_

extern const char kScheme[];
extern const char kDomain[];
extern const char kFileName[];
extern const int kSchemeRegistrationOptions;

// Create and register the custom scheme handler factory.
void RegisterSchemeHandlerFactory();

#endif
