﻿#pragma checksum "..\..\Administrator.xaml" "{8829d00f-11b8-4213-878b-770e8597ac16}" "19B90EF6ABC55AE6742799CE90D116D8D3B6137510418097E84621D43392B7E0"
//------------------------------------------------------------------------------
// <auto-generated>
//     Этот код создан программой.
//     Исполняемая версия:4.0.30319.42000
//
//     Изменения в этом файле могут привести к неправильной работе и будут потеряны в случае
//     повторной генерации кода.
// </auto-generated>
//------------------------------------------------------------------------------

using Airlanes;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Media.TextFormatting;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Shell;


namespace Airlanes {
    
    
    /// <summary>
    /// Administrator
    /// </summary>
    public partial class Administrator : System.Windows.Window, System.Windows.Markup.IComponentConnector {
        
        
        #line 11 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btnAddUser;
        
        #line default
        #line hidden
        
        
        #line 12 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btnExit;
        
        #line default
        #line hidden
        
        
        #line 14 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.DataGrid dataUsers;
        
        #line default
        #line hidden
        
        
        #line 38 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.ComboBox offices;
        
        #line default
        #line hidden
        
        
        #line 40 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button reset;
        
        #line default
        #line hidden
        
        
        #line 41 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button enable_disable;
        
        #line default
        #line hidden
        
        
        #line 42 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button delete;
        
        #line default
        #line hidden
        
        
        #line 43 "..\..\Administrator.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button changeRole;
        
        #line default
        #line hidden
        
        private bool _contentLoaded;
        
        /// <summary>
        /// InitializeComponent
        /// </summary>
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "4.0.0.0")]
        public void InitializeComponent() {
            if (_contentLoaded) {
                return;
            }
            _contentLoaded = true;
            System.Uri resourceLocater = new System.Uri("/Airlanes;component/administrator.xaml", System.UriKind.Relative);
            
            #line 1 "..\..\Administrator.xaml"
            System.Windows.Application.LoadComponent(this, resourceLocater);
            
            #line default
            #line hidden
        }
        
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "4.0.0.0")]
        [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Never)]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Design", "CA1033:InterfaceMethodsShouldBeCallableByChildTypes")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1800:DoNotCastUnnecessarily")]
        void System.Windows.Markup.IComponentConnector.Connect(int connectionId, object target) {
            switch (connectionId)
            {
            case 1:
            this.btnAddUser = ((System.Windows.Controls.Button)(target));
            
            #line 11 "..\..\Administrator.xaml"
            this.btnAddUser.Click += new System.Windows.RoutedEventHandler(this.btnAddUser_Click);
            
            #line default
            #line hidden
            return;
            case 2:
            this.btnExit = ((System.Windows.Controls.Button)(target));
            
            #line 12 "..\..\Administrator.xaml"
            this.btnExit.Click += new System.Windows.RoutedEventHandler(this.btnExit_Click);
            
            #line default
            #line hidden
            return;
            case 3:
            this.dataUsers = ((System.Windows.Controls.DataGrid)(target));
            return;
            case 4:
            this.offices = ((System.Windows.Controls.ComboBox)(target));
            
            #line 38 "..\..\Administrator.xaml"
            this.offices.SelectionChanged += new System.Windows.Controls.SelectionChangedEventHandler(this.offices_SelectionChanged);
            
            #line default
            #line hidden
            return;
            case 5:
            this.reset = ((System.Windows.Controls.Button)(target));
            
            #line 40 "..\..\Administrator.xaml"
            this.reset.Click += new System.Windows.RoutedEventHandler(this.reset_Click);
            
            #line default
            #line hidden
            return;
            case 6:
            this.enable_disable = ((System.Windows.Controls.Button)(target));
            
            #line 41 "..\..\Administrator.xaml"
            this.enable_disable.Click += new System.Windows.RoutedEventHandler(this.enable_disable_Click);
            
            #line default
            #line hidden
            return;
            case 7:
            this.delete = ((System.Windows.Controls.Button)(target));
            return;
            case 8:
            this.changeRole = ((System.Windows.Controls.Button)(target));
            
            #line 43 "..\..\Administrator.xaml"
            this.changeRole.Click += new System.Windows.RoutedEventHandler(this.changeRole_Click);
            
            #line default
            #line hidden
            return;
            }
            this._contentLoaded = true;
        }
    }
}

