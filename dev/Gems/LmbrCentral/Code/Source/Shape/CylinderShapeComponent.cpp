/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "LmbrCentral_precompiled.h"
#include "CylinderShapeComponent.h"
#include <AzCore/Math/IntersectPoint.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace LmbrCentral
{
    namespace ClassConverters
    {
        static bool DeprecateCylinderColliderConfiguration(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);
        static bool DeprecateCylinderColliderComponent(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement);
    }

    void CylinderShapeConfig::Reflect(AZ::ReflectContext* context)
    {
        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            // Deprecate: CylinderColliderConfiguration -> CylinderShapeConfig
            serializeContext->ClassDeprecate(
                "CylinderColliderConfiguration",
                "{E1DCB833-EFC4-43AC-97B0-4E07AA0DFAD9}",
                &ClassConverters::DeprecateCylinderColliderConfiguration
            );

            serializeContext->Class<CylinderShapeConfig>()
                ->Version(1)
                ->Field("Height", &CylinderShapeConfig::m_height)
                ->Field("Radius", &CylinderShapeConfig::m_radius)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<CylinderShapeConfig>("Configuration", "Cylinder shape configuration parameters")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ_CRC("PropertyVisibility_ShowChildrenOnly", 0xef428f20))
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CylinderShapeConfig::m_height, "Height", "Height of cylinder")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Suffix, " m")
                    ->Attribute(AZ::Edit::Attributes::Step, 0.1f)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CylinderShapeConfig::m_radius, "Radius", "Radius of cylinder")
                    ->Attribute(AZ::Edit::Attributes::Min, 0.f)
                    ->Attribute(AZ::Edit::Attributes::Suffix, " m")
                    ->Attribute(AZ::Edit::Attributes::Step, 0.05f)
                    ;
            }
        }

        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->Class<CylinderShapeConfig>()
                ->Property("Height", BehaviorValueProperty(&CylinderShapeConfig::m_height))
                ->Property("Radius", BehaviorValueProperty(&CylinderShapeConfig::m_radius));
        }

    }

    void CylinderShapeComponent::Reflect(AZ::ReflectContext* context)
    {
        CylinderShapeConfig::Reflect(context);

        auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->ClassDeprecate(
                "CylinderColliderComponent",
                "{A43F684B-07B6-4CD7-8D59-643709DF9486}",
                &ClassConverters::DeprecateCylinderColliderComponent
                );

            serializeContext->Class<CylinderShapeComponent, AZ::Component>()
                ->Version(1)
                ->Field("Configuration", &CylinderShapeComponent::m_configuration)
                ;
        }

        AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->Constant("CylinderShapeComponentTypeId", BehaviorConstant(CylinderShapeComponentTypeId));

            behaviorContext->EBus<CylinderShapeComponentRequestsBus>("CylinderShapeComponentRequestsBus")
                ->Event("GetCylinderConfiguration", &CylinderShapeComponentRequestsBus::Events::GetCylinderConfiguration)
                ->Event("SetHeight", &CylinderShapeComponentRequestsBus::Events::SetHeight)
                ->Event("SetRadius", &CylinderShapeComponentRequestsBus::Events::SetRadius)
                ;
        }
    }

    void CylinderShapeComponent::Activate()
    {
        CylinderShape::Activate(GetEntityId());
    }

    void CylinderShapeComponent::Deactivate()
    {
        CylinderShape::Deactivate();
    }

    bool CylinderShapeComponent::ReadInConfig(const AZ::ComponentConfig* baseConfig)
    {
        if (auto config = azrtti_cast<const CylinderShapeConfig*>(baseConfig))
        {
            m_configuration = *config;
            return true;
        }
        return false;
    }

    bool CylinderShapeComponent::WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const
    {
        if (auto outConfig = azrtti_cast<CylinderShapeConfig*>(outBaseConfig))
        {
            *outConfig = m_configuration;
            return true;
        }
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    namespace ClassConverters
    {
        static bool DeprecateCylinderColliderConfiguration(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement)
        {
            /*
            Old:
            <Class name="CylinderColliderConfiguration" field="Configuration" version="1" type="{E1DCB833-EFC4-43AC-97B0-4E07AA0DFAD9}">
             <Class name="float" field="Height" value="1.0000000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
             <Class name="float" field="Radius" value="0.2500000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
            </Class>

            New:
            <Class name="CylinderShapeConfig" field="Configuration" version="1" type="{53254779-82F1-441E-9116-81E1FACFECF4}">
             <Class name="float" field="Height" value="1.0000000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
             <Class name="float" field="Radius" value="0.2500000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
            </Class>
            */

            // Cache the Height and Radius
            float oldHeight = 0.f;
            float oldRadius = 0.f;

            int oldIndex = classElement.FindElement(AZ_CRC("Height", 0xf54de50f));
            if (oldIndex != -1)
            {
                classElement.GetSubElement(oldIndex).GetData<float>(oldHeight);
            }

            oldIndex = classElement.FindElement(AZ_CRC("Radius", 0x3b7c6e5a));
            if (oldIndex != -1)
            {
                classElement.GetSubElement(oldIndex).GetData<float>(oldRadius);
            }

            // Convert to CylinderShapeConfig
            bool result = classElement.Convert<CylinderShapeConfig>(context);
            if (result)
            {
                int newIndex = classElement.AddElement<float>(context, "Height");
                if (newIndex != -1)
                {
                    classElement.GetSubElement(newIndex).SetData<float>(context, oldHeight);
                }

                newIndex = classElement.AddElement<float>(context, "Radius");
                if (newIndex != -1)
                {
                    classElement.GetSubElement(newIndex).SetData<float>(context, oldRadius);
                }
                return true;
            }
            return false;
        }

        static bool DeprecateCylinderColliderComponent(AZ::SerializeContext& context, AZ::SerializeContext::DataElementNode& classElement)
        {
            /*
            Old:
            <Class name="CylinderColliderComponent" version="1" type="{A43F684B-07B6-4CD7-8D59-643709DF9486}">
             <Class name="CylinderColliderConfiguration" field="Configuration" version="1" type="{E1DCB833-EFC4-43AC-97B0-4E07AA0DFAD9}">
              <Class name="float" field="Height" value="1.0000000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
              <Class name="float" field="Radius" value="0.2500000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
             </Class>
            </Class>

            New:
            <Class name="CylinderShapeComponent" version="1" type="{B0C6AA97-E754-4E33-8D32-33E267DB622F}">
             <Class name="CylinderShapeConfig" field="Configuration" version="1" type="{53254779-82F1-441E-9116-81E1FACFECF4}">
              <Class name="float" field="Height" value="1.0000000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
              <Class name="float" field="Radius" value="0.2500000" type="{EA2C3E90-AFBE-44D4-A90D-FAAF79BAF93D}"/>
             </Class>
            </Class>
            */

            // Cache the Configuration
            CylinderShapeConfig configuration;
            int configIndex = classElement.FindElement(AZ_CRC("Configuration", 0xa5e2a5d7));
            if (configIndex != -1)
            {
                classElement.GetSubElement(configIndex).GetData<CylinderShapeConfig>(configuration);
            }

            // Convert to CylinderShapeComponent
            bool result = classElement.Convert<CylinderShapeComponent>(context);
            if (result)
            {
                configIndex = classElement.AddElement<CylinderShapeConfig>(context, "Configuration");
                if (configIndex != -1)
                {
                    classElement.GetSubElement(configIndex).SetData<CylinderShapeConfig>(context, configuration);
                }
                return true;
            }
            return false;
        }

    } // namespace ClassConverters

} // namespace LmbrCentral
