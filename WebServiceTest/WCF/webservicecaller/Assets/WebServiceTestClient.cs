// ------------------------------------------------------------------------------
//  <autogenerated>
//      This code was generated by a tool.
//      Mono Runtime Version: 2.0.50727.1433
// 
//      Changes to this file may cause incorrect behavior and will be lost if 
//      the code is regenerated.
//  </autogenerated>
// ------------------------------------------------------------------------------




[System.ServiceModel.ServiceContractAttribute(Namespace="http://tempuri.org/")]
public interface IWebServiceTest {
    
    [System.ServiceModel.OperationContractAttribute(Action="http://tempuri.org/IWebServiceTest/Hello", ReplyAction="http://tempuri.org/IWebServiceTest/HelloResponse")]
    string Hello(string name);
}

public interface IWebServiceTestChannel : IWebServiceTest, System.ServiceModel.IClientChannel {
}

public class WebServiceTestClient : System.ServiceModel.ClientBase<IWebServiceTest>, IWebServiceTest {
    
    public WebServiceTestClient() {
    }
    
    public WebServiceTestClient(string endpointConfigurationName) : 
            base(endpointConfigurationName) {
    }
    
    public WebServiceTestClient(string endpointConfigurationName, string remoteAddress) : 
            base(endpointConfigurationName, remoteAddress) {
    }
    
    public WebServiceTestClient(string endpointConfigurationName, System.ServiceModel.EndpointAddress remoteAddress) : 
            base(endpointConfigurationName, remoteAddress) {
    }
    
    public WebServiceTestClient(System.ServiceModel.Channels.Binding binding, System.ServiceModel.EndpointAddress endpoint) : 
            base(binding, endpoint) {
    }
    
    public string Hello(string name) {
        return base.Channel.Hello(name);
    }
}



