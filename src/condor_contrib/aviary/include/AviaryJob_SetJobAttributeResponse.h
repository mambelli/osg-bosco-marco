

        #ifndef AviaryJob_SETJOBATTRIBUTERESPONSE_H
        #define AviaryJob_SETJOBATTRIBUTERESPONSE_H

       /**
        * SetJobAttributeResponse.h
        *
        * This file was auto-generated from WSDL
        * by the Apache Axis2/Java version: 1.0  Built on : Mar 02, 2011 (11:54:00 EST)
        */

       /**
        *  SetJobAttributeResponse class
        */

        namespace AviaryJob{
            class SetJobAttributeResponse;
        }
        

        
       #include "AviaryJob_ControlJobResponse.h"
          
        #include <axutil_qname.h>
        

        #include <stdio.h>
        #include <OMElement.h>
        #include <ServiceClient.h>
        #include <ADBDefines.h>

namespace AviaryJob
{
        
        

        class SetJobAttributeResponse {

        private:
             
                axutil_qname_t* qname;
            AviaryJob::ControlJobResponse* property_SetJobAttributeResponse;

                
                bool isValidSetJobAttributeResponse;
            

        /*** Private methods ***/
          

        bool WSF_CALL
        setSetJobAttributeResponseNil();
            



        /******************************* public functions *********************************/

        public:

        /**
         * Constructor for class SetJobAttributeResponse
         */

        SetJobAttributeResponse();

        /**
         * Destructor SetJobAttributeResponse
         */
        ~SetJobAttributeResponse();


       

        /**
         * Constructor for creating SetJobAttributeResponse
         * @param 
         * @param SetJobAttributeResponse AviaryJob::ControlJobResponse*
         * @return newly created SetJobAttributeResponse object
         */
        SetJobAttributeResponse(AviaryJob::ControlJobResponse* arg_SetJobAttributeResponse);
        
        
        /********************************** Class get set methods **************************************/
        
        

        /**
         * Getter for SetJobAttributeResponse. 
         * @return AviaryJob::ControlJobResponse*
         */
        WSF_EXTERN AviaryJob::ControlJobResponse* WSF_CALL
        getSetJobAttributeResponse();

        /**
         * Setter for SetJobAttributeResponse.
         * @param arg_SetJobAttributeResponse AviaryJob::ControlJobResponse*
         * @return true on success, false otherwise
         */
        WSF_EXTERN bool WSF_CALL
        setSetJobAttributeResponse(AviaryJob::ControlJobResponse*  arg_SetJobAttributeResponse);

        /**
         * Re setter for SetJobAttributeResponse
         * @return true on success, false
         */
        WSF_EXTERN bool WSF_CALL
        resetSetJobAttributeResponse();
        


        /******************************* Checking and Setting NIL values *********************************/
        

        /**
         * NOTE: set_nil is only available for nillable properties
         */

        

        /**
         * Check whether SetJobAttributeResponse is Nill
         * @return true if the element is Nil, false otherwise
         */
        bool WSF_CALL
        isSetJobAttributeResponseNil();


        

        /**************************** Serialize and De serialize functions ***************************/
        /*********** These functions are for use only inside the generated code *********************/

        
        /**
         * Deserialize the ADB object to an XML
         * @param dp_parent double pointer to the parent node to be deserialized
         * @param dp_is_early_node_valid double pointer to a flag (is_early_node_valid?)
         * @param dont_care_minoccurs Dont set errors on validating minoccurs, 
         *              (Parent will order this in a case of choice)
         * @return true on success, false otherwise
         */
        bool WSF_CALL
        deserialize(axiom_node_t** omNode, bool *isEarlyNodeValid, bool dontCareMinoccurs);
                         
            

       /**
         * Declare namespace in the most parent node 
         * @param parent_element parent element
         * @param namespaces hash of namespace uri to prefix
         * @param next_ns_index pointer to an int which contain the next namespace index
         */
        void WSF_CALL
        declareParentNamespaces(axiom_element_t *parent_element, axutil_hash_t *namespaces, int *next_ns_index);


        

        /**
         * Serialize the ADB object to an xml
         * @param SetJobAttributeResponse_om_node node to serialize from
         * @param SetJobAttributeResponse_om_element parent element to serialize from
         * @param tag_closed Whether the parent tag is closed or not
         * @param namespaces hash of namespace uris to prefixes
         * @param next_ns_index an int which contains the next namespace index
         * @return axiom_node_t on success,NULL otherwise.
         */
        axiom_node_t* WSF_CALL
        serialize(axiom_node_t* SetJobAttributeResponse_om_node, axiom_element_t *SetJobAttributeResponse_om_element, int tag_closed, axutil_hash_t *namespaces, int *next_ns_index);

        /**
         * Check whether the SetJobAttributeResponse is a particle class (E.g. group, inner sequence)
         * @return true if this is a particle class, false otherwise.
         */
        bool WSF_CALL
        isParticle();



        /******************************* get the value by the property number  *********************************/
        /************NOTE: This method is introduced to resolve a problem in unwrapping mode *******************/

      
        

        /**
         * Getter for SetJobAttributeResponse by property number (1)
         * @return AviaryJob::ControlJobResponse
         */

        AviaryJob::ControlJobResponse* WSF_CALL
        getProperty1();

    

};

}        
 #endif /* SETJOBATTRIBUTERESPONSE_H */
    

